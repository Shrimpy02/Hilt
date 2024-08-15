#include "Components/GrapplingHook/GrapplingComponent.h"

#include "Components/CapsuleComponent.h"
#include "Core/HiltTags.h"
#include "NPC/Components/GrappleableComponent.h"
#include "Components/PlayerMovementComponent.h"
#include "Components/Camera/PlayerCameraComponent.h"
#include "Components/GrapplingHook/RopeComponent.h"
#include "Player/PlayerCharacter.h"
#include "Player/ScoreComponent.h"

FGrappleInterpStruct::FGrappleInterpStruct(const float InPullSpeed, const float InPullAccel, const EInterpToTargetType InInterpMode): PullSpeed(InPullSpeed), PullAccel(InPullAccel), InInterpMode(InInterpMode)
{

}

UGrapplingComponent::UGrapplingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;
}

void UGrapplingComponent::BeginPlay()
{
	Super::BeginPlay();

	//get the rope component
	RopeComponent = GetOwner()->FindComponentByClass<URopeComponent>();

	//get the owner as a player character
	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());

	//setup start and stop grapple events for the rope component
	OnStartGrapple.AddDynamic(RopeComponent, &URopeComponent::ActivateRope);
	OnStopGrapple.AddDynamic(RopeComponent, &URopeComponent::DeactivateRope);
}

void UGrapplingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//call the parent implementation
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//update the can grapple variable
	CanGrappleVar = CanGrapple(false);

	//check if we're grappling
	if (bIsGrappling)
	{
		//assign the grapple direction
		GrappleDirection = RopeComponent->GetRopeDirection(GrappleDirectionChecks).GetSafeNormal();

		//apply the pull force
		ApplyPullForce(DeltaTime);

		//check that we're not grounded
		if (!PlayerCharacter->PlayerMovementComponent->IsMovingOnGround() && -GrappleDirection.GetSafeNormal().Z < PlayerCharacter->PlayerMovementComponent->GetWalkableFloorZ())
		{
			//set borientrotationtoMovement to false
			PlayerCharacter->PlayerMovementComponent->bOrientRotationToMovement = false;

			//set the rotation of the character to the grapple direction
			GetOwner()->SetActorRotation(GrappleDirection.Rotation());
		}
		else
		{
			//set borientrotationtoMovement to true
			PlayerCharacter->PlayerMovementComponent->bOrientRotationToMovement = true;
		}

		//check if the grapplescorecurve is valid
		if (GrappleScoreCurve)
		{
			//get the grapple score curve value
			const float Value = GrappleScoreCurve->GetFloatValue(GetWorld()->GetTimeSeconds() - GrappleStartTime);

			//set the pending score
			PendingScore = Value * PlayerCharacter->ScoreComponent->GetCurrentScoreValues().ScoreGainMultiplier;
		}

		//check if the grapple start time + GrappleScoreDecayStopDelay is less than the current time
		if (GrappleStartTime + GrappleScoreDecayStopDelay < GetWorld()->GetTimeSeconds())
		{
			//stop the score degredation timer
			PlayerCharacter->ScoreComponent->StopDegredationTimer();
		}

		//check if we should stop grappling
		StopGrappleCheck();
	}
	else
	{
		////set the grapple direction to zero if we're not grappling
		//GrappleDirection = FVector::ZeroVector;
	}
}

void UGrapplingComponent::StartGrapple(const FHitResult& HitResult)
{
	//check if we're already grappling
	if (bIsGrappling)
	{
		//stop grappling
		StopGrapple();
	}

	//check if the other actor is valid
	if (HitResult.GetActor()->IsValidLowLevelFast())
	{
		//bind the destroyed event to this component
		HitResult.GetActor()->OnDestroyed.AddDynamic(this, &UGrapplingComponent::OnGrappleTargetDestroyed);

		//set the grapple target
		GrappleTarget = HitResult.GetActor();
	}
	//check if the hit result is valid
	else if (HitResult.bBlockingHit)
	{
		//bind the destroyed event to this component
		HitResult.GetActor()->OnDestroyed.AddDynamic(this, &UGrapplingComponent::OnGrappleTargetDestroyed);

		//set the grapple target
		GrappleTarget = HitResult.GetActor();
	}
	else
	{
		//somethin went wrong, print an error message
		UE_LOG(LogTemp, Error, TEXT("Something went wrong when trying to start the grapple"));

		//return early
		return;
	}

	//check if the rope component is valid
	if (RopeComponent->IsValidLowLevelFast())
	{
		//activate the rope component
		RopeComponent->ActivateRope(HitResult);
	}

	//update the grapple direction (done immediately to for the animation blueprint)
	GrappleDirection = RopeComponent->GetRopeDirection(GrappleDirectionChecks).GetSafeNormal();

	//update bIsGrappling
	bIsGrappling = true;

	//check if the other actor has a grappleable component
	if (GrappleableComponent = HitResult.GetActor()->GetComponentByClass<UGrappleableComponent>(); GrappleableComponent->IsValidLowLevelFast())
	{
		//set bisgrappled to true
		GrappleableComponent->bIsGrappled = true;

		//bind the events to this component
		OnStartGrapple.AddDynamic(GrappleableComponent, &UGrappleableComponent::OnStartGrapple);
		OnStopGrapple.AddDynamic(GrappleableComponent, &UGrappleableComponent::OnStopGrapple);
	}
	//check if we shouldn't use normal movement
	if (!ShouldUseNormalMovement())
	{
		//set the movement mode to falling to prevent us from being stuck on the ground
		PlayerCharacter->PlayerMovementComponent->SetMovementMode(MOVE_Falling);

		//raise the walkable floor angle by 10
		PlayerCharacter->PlayerMovementComponent->SetWalkableFloorAngle(PlayerCharacter->PlayerMovementComponent->GetWalkableFloorAngle() + 30);
	}

	//check if we should disable gravity when grappling
	if (!bApplyGravityWhenGrappling)
	{
		//disable gravity
		GetOwner()->FindComponentByClass<UPrimitiveComponent>()->SetEnableGravity(false);
	}
	else
	{
		//set the gravity scale back to normal
		PlayerCharacter->PlayerMovementComponent->GravityScale = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GravityScale;
	}

	GrappleStartTime = GetWorld()->GetTimeSeconds();

	//call the OnStartGrapple event
	OnStartGrapple.Broadcast(HitResult);
}

void UGrapplingComponent::StopGrapple(bool CallBlueprintEvent)
{
	//check if we're not grappling
	if (!bIsGrappling)
	{
		//return early
		return;
	}

	//check if the grapple target is valid
	if (GrappleTarget->IsValidLowLevelFast())
	{
		//unbind the destroyed event from the grapple target
		GrappleTarget->OnDestroyed.RemoveDynamic(this, &UGrapplingComponent::OnGrappleTargetDestroyed);
	}

	//check if the rope component is valid
	if (RopeComponent->IsValidLowLevelFast())
	{
		//deactivate the rope component
		RopeComponent->DeactivateRope();
	}

	//update bIsGrappling
	bIsGrappling = false;

	//check if the grapplescorecurve is valid
	if (GrappleScoreCurve)
	{
		//get the grapple score curve value
		const float Value = GrappleScoreCurve->GetFloatValue(GetWorld()->GetTimeSeconds() - GrappleStartTime);

		//set the pending score to 0
		PendingScore = 0;

		//add the grapple score curve value to the player's score
		PlayerCharacter->ScoreComponent->AddScore(Value);
	}

	//check if we should call the blueprint event
	if (CallBlueprintEvent)
	{
		//call the OnStopGrapple event
		OnStopGrapple.Broadcast();
	}

	//check if we have a grappleable component
	if (GrappleableComponent->IsValidLowLevelFast())
	{
		//set bisgrappled to false
		GrappleableComponent->bIsGrappled = false;

		//unbind the events from this component
		OnStartGrapple.RemoveDynamic(GrappleableComponent, &UGrappleableComponent::OnStartGrapple);
		OnStopGrapple.RemoveDynamic(GrappleableComponent, &UGrappleableComponent::OnStopGrapple);
	}

	//reset the owner's rotation
	GetOwner()->SetActorRotation(PlayerCharacter->PlayerMovementComponent->Velocity.Rotation());

	//check if we should reenable gravity
	if (!bApplyGravityWhenGrappling)
	{
		//enable gravity
		PlayerCharacter->GetCapsuleComponent()->SetEnableGravity(true);
	}
	else
	{
		//set the gravity scale back to normal
		PlayerCharacter->PlayerMovementComponent->GravityScale = PlayerCharacter->PlayerMovementComponent->DefaultGravityScale;
	}

	//check if we shouldn't use normal movement
	if (!ShouldUseNormalMovement())
	{
		//set the walkable floor angle back to normal
		PlayerCharacter->PlayerMovementComponent->SetWalkableFloorAngle(PlayerCharacter->PlayerMovementComponent->GetWalkableFloorAngle() - 30);
	}

	//set borientrotationtoMovement to true
	PlayerCharacter->PlayerMovementComponent->bOrientRotationToMovement = true;
}

void UGrapplingComponent::StartGrappleCheck()
{
	//check if we can grapple and we're not already grappling
	if (CanGrapple(true) && !bIsGrappling)
	{
		////do a line trace to see if the player is aiming at something within grapple range
		//TArray<FHitResult> GrappleHits;

		//DoGrappleTrace(GrappleHits, MaxGrappleCheckDistance, false);

		//check if the line trace hit something
		if (!GrappleHits.IsEmpty())
		{
			//check if the distance between the trace start and the hit location is less than the distance between the trace start and the player character
			if (FVector::Dist(GetOwner()->GetActorLocation(), GrappleHits[0].ImpactPoint) < FVector::Dist(GetOwner()->GetActorLocation(), GrappleHits[0].TraceStart))
			{
				return;
			}

			//start grappling
			StartGrapple(GrappleHits[0]);
		}
	}
}

void UGrapplingComponent::StopGrappleCheck()
{
	//check if we're close to the end of the rope
	if (FVector::Dist(GetOwner()->GetActorLocation(), RopeComponent->GetRopeEnd()) < GrappleStopDistance)
	{
		//stop grappling
		StopGrapple();
	}
}

FVector UGrapplingComponent::ProcessGrappleInput(FVector MovementInput)
{
	GrappleInput = MovementInput;

	//check that the player is grappling
	if (!bIsGrappling || ShouldUseNormalMovement())
	{
		return MovementInput;
	}

	//check if the grapple input is zero
	if (GrappleInput.IsNearlyZero())
	{
		//check if the input vector is nearly zero
		SetGrappleMode(InterpVelocity);
	}
	else
	{
		//set the grapple mode to add to velocity
		SetGrappleMode(AddToVelocity);
	}

	//storage for the return vector
	FVector ReturnVec = MovementInput * GrappleMovementInputModifier * PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrapplingInputModifier;

	//check if we have valid angle input curve
	if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementAngleInputCurve)
	{
		//get the dot product of the current grapple direction and the return vector
		const float DotProduct = FVector::DotProduct(PlayerCharacter->GetActorUpVector(),MovementInput.GetSafeNormal());

		//get the grapple angle movement input curve value
		const float Value = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementAngleInputCurve->GetFloatValue(DotProduct);

		//multiply the return vector
		ReturnVec *= Value;
	}

	//check if we have a valid grapple movement distance curve
	if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementDistanceInputCurve)
	{
		//get the grapple distance movement input curve value
		const float Value = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementDistanceInputCurve->GetFloatValue(FMath::Clamp(FVector::Dist(GetOwner()->GetActorLocation(), RopeComponent->GetRopeEnd()) / MaxGrappleDistance, 0, 1));

		//multiply the return 
		ReturnVec *= Value;
	}

	//check if we have a valid GrappleMovementSpeedCurve
	if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementSpeedCurve)
	{
		//get the grapple velocity movement input curve value
		const float Value = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementSpeedCurve->GetFloatValue(PlayerCharacter->PlayerMovementComponent->ApplySpeedLimit(ReturnVec, DELTA, false).Size() / PlayerCharacter->PlayerMovementComponent->GetCurrentSpeedLimit());

		//multiply the return vector
		ReturnVec *= Value;
	}

	//check if we have a valid GrappleMovementDirectionCurve
	if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementDirectionCurve)
	{
		//get the grapple direction movement input curve value
		const float Value = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementDirectionCurve->GetFloatValue(FVector::DotProduct(ReturnVec.GetSafeNormal(), PlayerCharacter->PlayerMovementComponent->Velocity.GetSafeNormal()));

		//multiply the return vector
		ReturnVec *= Value;
	}

	//apply the speed limit to the return vector
	ReturnVec = PlayerCharacter->PlayerMovementComponent->ApplySpeedLimit(ReturnVec, DELTA, false);

	//return the return vector
	return ReturnVec;
}

bool UGrapplingComponent::ShouldUseNormalMovement() const
{
	//check if we have a valid grappleable component
	if (GrappleableComponent->IsValidLowLevelFast())
	{
		//check if the grappleable component has normal movement enabled
		if (GrappleableComponent->NormalMovement)
		{
			return true;
		}
	}

	//otherwise return whether or not we're using debug mode
	return bUseDebugMode;
}

void UGrapplingComponent::DoInterpGrapple(float DeltaTime, FVector& GrappleVelocity, FGrappleInterpStruct GrappleInterpStruct)
{
	//storage for the grapple direction
	const FVector LocGrappleDirection = GrappleDirection.GetSafeNormal();

	//switch on the interp mode
	switch (GrappleInterpStruct.InterpMode)
	{
		case InterpTo:
			//interpolate the velocity
			GrappleVelocity = PlayerCharacter->PlayerMovementComponent->ApplySpeedLimit(FMath::VInterpTo(GetOwner()->GetVelocity(), LocGrappleDirection * GrappleInterpStruct.PullSpeed, DeltaTime, GrappleInterpStruct.PullAccel), DeltaTime);
			break;
		case InterpStep:
			//interpolate the velocity
			GrappleVelocity = PlayerCharacter->PlayerMovementComponent->ApplySpeedLimit(FMath::VInterpTo(GetOwner()->GetVelocity(), LocGrappleDirection * GrappleInterpStruct.PullSpeed, DeltaTime, GrappleInterpStruct.PullAccel), DeltaTime);
			break;

		default /* constant */:
			//interpolate the velocity
			GrappleVelocity = PlayerCharacter->PlayerMovementComponent->ApplySpeedLimit(FMath::VInterpConstantTo(GetOwner()->GetVelocity(),  LocGrappleDirection * GrappleInterpStruct.PullSpeed, DeltaTime, GrappleInterpStruct.PullAccel), DeltaTime);
			break;
	}

	//check for potential modifiers to the grapple velocity from the grappleable component
	CheckTargetForceModifiers(GrappleVelocity, DeltaTime);

	//calculate the grapple dot product
	GrappleDotProduct = GetGrappleDotProduct(GrappleVelocity);

	//calculate the absolute grapple dot product
	AbsoluteGrappleDotProduct = GetAbsoluteGrappleDotProduct(GrappleVelocity);
}

void UGrapplingComponent::DoGrappleTrace(float MaxDistance, bool DoSphereTrace)
{
	//storage for camera location and rotation
	FVector CameraLocation;
	FRotator CameraRotation;

	//set the camera location and rotation
	GetOwner()->GetNetOwningPlayer()->GetPlayerController(GetWorld())->GetPlayerViewPoint(CameraLocation, CameraRotation);

	//get the forward vector of the camera rotation
	const FVector Rotation = CameraRotation.Quaternion().GetForwardVector();

	//get the end point of the line trace
	const FVector End = CameraLocation + Rotation * MaxDistance;

	//the collision parameters to use for the line trace
	const FCollisionQueryParams GrappleCollisionParams = RopeComponent->GetCollisionParams();

	//empty the grapple trace hits
	GrappleHits.Empty();

	//storage for the temp array
	TArray<FHitResult> TempArray;

	//do the line trace
	GetWorld()->LineTraceMultiByChannel(TempArray, CameraLocation, End, RopeComponent->CollisionChannel, GrappleCollisionParams);

	//check if the temp array is empty and we're doing a sphere trace
	if (TempArray.IsEmpty() && DoSphereTrace)
	{
		//do a sphere multi trace
		GetWorld()->SweepMultiByChannel(TempArray, CameraLocation, End, FQuat::Identity, RopeComponent->CollisionChannel, FCollisionShape::MakeSphere(GrappleSphereRadius), GrappleCollisionParams);

		//print a debug message to the screen
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Sphere Trace"));
	}

	for (const FHitResult& GrappleHit : TempArray)
	{
		//get the distance from the line trace start to the hit location
		const float Distance = FVector::Dist(GetOwner()->GetActorLocation(), GrappleHit.ImpactPoint);

		//check if the distance between the trace start and the hit location is less than the distance between the trace start and the player character
		if (Distance - GrappleCheckWiggleRoom < FVector::Dist(GetOwner()->GetActorLocation(), GrappleHit.TraceStart) || GrappleHit.bStartPenetrating)
		{
			continue;
		}

		//add the hit to the returned hits
		GrappleHits.Add(GrappleHit);
	}
}

void UGrapplingComponent::CheckTargetForceModifiers(FVector& BaseVel, float DeltaTime) const
{
	//check if we have a valid grappleable component
	if (GrappleableComponent->IsValidLowLevelFast())
	{
		//apply the grapple velocity to the owner of the grappleable component
		Cast<UPrimitiveComponent>(GrappleableComponent->GetOwner()->GetRootComponent())->SetAllPhysicsLinearVelocity(FMath::VInterpTo(GrappleableComponent->GetOwner()->GetVelocity(), -GrappleDirection * GrappleableComponent->GrappleInterpStructThis.PullSpeed, DeltaTime, GrappleableComponent->GrappleInterpStructThis.PullAccel), false);

		//apply the grapple velocity to the player
		BaseVel *= GrappleableComponent->GrappleReelForceMultiplierPlayer;
	}
}

void UGrapplingComponent::ApplyPullForce(float DeltaTime)
{
	//check if we're using debug mode
	if (ShouldUseNormalMovement())
	{
		//return early
		return;
	}

	//storage for the velocity that will be applied from the grapple
	FVector GrappleVelocity = /*RopeComponent->GetRopeDirection(0)*/ GrappleDirection.GetSafeNormal() * GetPullSpeed() * DeltaTime;

	FVector BaseVel;

	//check how we should set the velocity
	// ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
	switch (GetGrappleMode())
	{
		case AddToVelocity:
			//check if we have a valid angle curve
			if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleAngleCurve)
			{
				//get the grapple angle curve value
				const float Value = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleAngleCurve->GetFloatValue(GetGrappleDotProduct(GrappleVelocity.GetSafeNormal()));

				//multiply the grapple velocity by the grapple curve value
				GrappleVelocity *= Value;
			}

			//check if we have a valid distance curve
			if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleDistanceCurve)
			{
				//get the grapple distance curve value
				const float Value = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleDistanceCurve->GetFloatValue(FMath::Clamp(FVector::Dist(GetOwner()->GetActorLocation(), RopeComponent->GetRopeEnd()) / MaxGrappleDistance, 0, 1));
				 
				//multiply the grapple velocity by the grapple distance curve value
				GrappleVelocity *= Value;
			}

			//check if we have a valid GrappleVelocityCurve and GrappleVelocityDotProductCurve
			if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleVelocityCurve)
			{
				//get the grapple velocity curve value
				const float VelocityValue = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleVelocityCurve->GetFloatValue(PlayerCharacter->PlayerMovementComponent->ApplySpeedLimit(GrappleVelocity, DeltaTime, false).Size() / PlayerCharacter->PlayerMovementComponent->GetCurrentSpeedLimit());
				
				//multiply the grapple velocity by the grapple velocity curve value
				GrappleVelocity *= VelocityValue;
			}

			//calculate the grapple dot product
			GrappleDotProduct = GetGrappleDotProduct(GrappleVelocity);

			//calculate the absolute grapple dot product
			AbsoluteGrappleDotProduct = GetAbsoluteGrappleDotProduct(GrappleVelocity);

			BaseVel = PlayerCharacter->PlayerMovementComponent->Velocity + GrappleVelocity;

			//apply potential modifiers to the grapple velocity from the grappleable component
			CheckTargetForceModifiers(BaseVel, DeltaTime);

			//apply the grapple velocity
			PlayerCharacter->PlayerMovementComponent->Velocity = PlayerCharacter->PlayerMovementComponent->ApplySpeedLimit(BaseVel, DeltaTime, false);

		break;
		case InterpVelocity:
			//do the interpolation
			DoInterpGrapple(DeltaTime, PlayerCharacter->PlayerMovementComponent->Velocity, GetGrappleInterpStruct());
		break;
	}
}

void UGrapplingComponent::OnGrappleTargetDestroyed(AActor* DestroyedActor)
{
	//stop grappling
	StopGrapple();
}

FGrappleInterpStruct UGrapplingComponent::GetGrappleInterpStruct() const
{
	//check if we have a valid grappleable component
	if (GrappleableComponent)
	{
		//check if we should use the grappleable components grapple interp struct
		if (GrappleableComponent->ShouldUseGrappleInterpStruct())
		{
			//return the objective grapple interp struct
			return GrappleableComponent->GetGrappleInterpStruct();
		}
	}

	//default to the NoWasd grapple interp struct
	return NoWasdGrappleInterpStruct;
}

float UGrapplingComponent::GetPullSpeed() const
{
	//check if we're in the AddToVelocity grapple mode
	if (GrappleMode == AddToVelocity)
	{
		//return the pull speed from the objective grapple interp struct
		return GetGrappleInterpStruct().PullSpeed * PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleSpeedMultiplier;
	}
	//check if we're in the InterpVelocity grapple mode
	if (GrappleMode == InterpVelocity)
	{
		//todo check if this works
		//return the pull speed from the objective grapple interp struct
		return GetGrappleInterpStruct().PullSpeed * PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleSpeedMultiplier;
	}

	//return 0
	return 0.f;
}

TEnumAsByte<EGrapplingMode> UGrapplingComponent::GetGrappleMode() const
{
	return GrappleMode;
}

float UGrapplingComponent::GetGrappleDotProduct(FVector GrappleVelocity) const
{
	//get the dot product of the owner's velocity and the grapple velocity
	return FVector::DotProduct(GetOwner()->GetVelocity().GetSafeNormal(), GrappleVelocity.GetSafeNormal());
}

float UGrapplingComponent::GetAbsoluteGrappleDotProduct(FVector GrappleVelocity)
{
	//get the dot product of the grapple direction and (0, 0, 1)
	return FVector::DotProduct(GrappleVelocity.GetSafeNormal(), FVector(0, 0, 1));
}

bool UGrapplingComponent::CanGrapple(const bool DoSphereTrace)
{
	//do a line trace to see if the player is aiming at something within grapple range
	DoGrappleTrace(MaxGrappleDistance, DoSphereTrace);

	////check if the line trace didn't hit anything
	//if (!GrappleHit.bBlockingHit)
	//{
	//	//return false
	//	return false;
	//}

	////check if the line trace hit something ungrappleable
	//if (GrappleHit.GetActor()->ActorHasTag(HiltTags::NoGrappleTag) || GrappleHit.GetComponent()->ComponentHasTag(HiltTags::NoGrappleTag))
	//{
	//	//return false
	//	return false;
	//}

	//return true
	return !GrappleHits.IsEmpty();
}

float UGrapplingComponent::GetRemainingGrappleDistance()
{
	//do the grapple trace
	DoGrappleTrace(MaxGrappleCheckDistance, false);

	//check if the line trace hit something
	if (!GrappleHits.IsEmpty())
	{
		//if the line trace hit something, return the distance to the hit
		if (GrappleHits[0].bBlockingHit)
		{
			//get the distance left until the player can grapple to where they are aiming and check if it's greater than 0
			if (const float GrappleDistanceLeft = FVector::Dist(GetOwner()->GetActorLocation(), GrappleHits[0].ImpactPoint) - MaxGrappleDistance; GrappleDistanceLeft > 0.f)
			{
				//return the distance left until the player can grapple to where they are aiming
				return GrappleDistanceLeft;
			}
		}
	}

	//otherwise return 0
	return 0.f;
}

void UGrapplingComponent::SetGrappleMode(const TEnumAsByte<EGrapplingMode> NewGrappleMode)
{
	//check if we have a valid grappleable component
	if (GrappleableComponent)
	{
		//check if we can't change the grapple mode
		if (!GrappleableComponent->CanChangeGrappleMode())
		{
			//prevent changing the grapple mode
			return;
		}
	}

	//set the new grapple mode
	GrappleMode = NewGrappleMode;
}
