#include "Components/GrapplingHook/GrapplingComponent.h"
#include "Core/HiltTags.h"
#include "NPC/Components/GrappleableComponent.h"
#include "Components/PlayerMovementComponent.h"
#include "Components/GrapplingHook/RopeComponent.h"

FGrappleInterpStruct::FGrappleInterpStruct(const float InPullSpeed, const float InPullAccel, const EInterpToTargetType InInterpMode): InInterpMode(InInterpMode), PullSpeed(InPullSpeed), PullAccel(InPullAccel)
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

	//get the player movement component
	PlayerMovementComponent = GetOwner()->FindComponentByClass<UPlayerMovementComponent>();

	//setup start and stop grapple events for the rope component
	OnStartGrapple.AddDynamic(RopeComponent, &URopeComponent::ActivateRope);
	OnStopGrapple.AddDynamic(RopeComponent, &URopeComponent::DeactivateRope);
}

void UGrapplingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//call the parent implementation
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//set all grappling variables to their default values
	GrappleDirection = FVector::ZeroVector;
	GrappleDotProduct = 0.f;
	AbsoluteGrappleDotProduct = 0.f;

	//update the can grapple variable
	CanGrappleVar = CanGrapple();

	//check if we're grappling
	if (bIsGrappling)
	{
		//call the WhileGrappled event
		WhileGrappled.Broadcast(DeltaTime);

		//apply the pull force
		ApplyPullForce(DeltaTime);

		//assign the grapple direction
		GrappleDirection = GetGrappleDirection();

		//check that we're not grounded
		if (!PlayerMovementComponent->IsMovingOnGround() && -GrappleDirection.GetSafeNormal().Z < PlayerMovementComponent->GetWalkableFloorZ())
		{
			//set borientrotationtoMovement to false
			PlayerMovementComponent->bOrientRotationToMovement = false;

			//set the rotation of the character to the grapple direction
			GetOwner()->SetActorRotation(GetGrappleDirection().Rotation());
		}
		else
		{
			//set borientrotationtoMovement to true
			PlayerMovementComponent->bOrientRotationToMovement = true;
		}
	}
}

void UGrapplingComponent::StartGrapple(AActor* OtherActor, const FHitResult& HitResult)
{
	//check if we're already grappling
	if (bIsGrappling)
	{
		//stop grappling
		StopGrapple();
	}

	//check if we're not using debug mode
	if (!bUseDebugMode)
	{
		//set the movement mode to falling to prevent us from being stuck on the ground
		PlayerMovementComponent->SetMovementMode(MOVE_Falling);
	}

	//check if the other actor is valid
	if (OtherActor->IsValidLowLevelFast())
	{
		//bind the destroyed event to this component
		OtherActor->OnDestroyed.AddDynamic(this, &UGrapplingComponent::OnGrappleTargetDestroyed);

		//set the grapple target
		GrappleTarget = OtherActor;
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
		RopeComponent->ActivateRope(OtherActor, HitResult);
	}

	//update the grapple direction (done immediately to for the animation blueprint)
	GrappleDirection = GetGrappleDirection();

	//update bIsGrappling
	bIsGrappling = true;

	//check if the other actor has a grappleable component
	if (GrappleableComponent = OtherActor->GetComponentByClass<UGrappleableComponent>(); GrappleableComponent->IsValidLowLevelFast())
	{
		//bind the events to this component
		OnStartGrapple.AddDynamic(GrappleableComponent, &UGrappleableComponent::OnStartGrapple);
		OnStopGrapple.AddDynamic(GrappleableComponent, &UGrappleableComponent::OnStopGrapple);
		WhileGrappled.AddDynamic(GrappleableComponent, &UGrappleableComponent::WhileGrappled);
	}

	//check if we should disable gravity when grappling
	if (bDisableGravityWhenGrappling)
	{
		//disable gravity
		GetOwner()->FindComponentByClass<UPrimitiveComponent>()->SetEnableGravity(false);
	}

	//call the OnStartGrapple event
	OnStartGrapple.Broadcast(OtherActor, HitResult);
}

void UGrapplingComponent::StopGrapple()
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

	//update the grapple direction (done immediately to for the animation blueprint)
	GrappleDirection = FVector::ZeroVector;

	//check if the rope component is valid
	if (RopeComponent->IsValidLowLevelFast())
	{
		//deactivate the rope component
		RopeComponent->DeactivateRope();
	}

	//update bIsGrappling
	bIsGrappling = false;

	//call the OnStopGrapple event
	OnStopGrapple.Broadcast();

	//check if we have a grappleable component
	if (GrappleableComponent->IsValidLowLevelFast())
	{
		//unbind the events from this component
		OnStartGrapple.RemoveDynamic(GrappleableComponent, &UGrappleableComponent::OnStartGrapple);
		OnStopGrapple.RemoveDynamic(GrappleableComponent, &UGrappleableComponent::OnStopGrapple);
		WhileGrappled.RemoveDynamic(GrappleableComponent, &UGrappleableComponent::WhileGrappled);
	}

	//reset the owner's rotation
	GetOwner()->SetActorRotation(PlayerMovementComponent->Velocity.Rotation());

	//check if we should disable gravity when grappling
	if (bDisableGravityWhenGrappling)
	{
		//enable gravity
		GetOwner()->FindComponentByClass<UPrimitiveComponent>()->SetEnableGravity(true);
	}

	//set borientrotationtoMovement to true
	PlayerMovementComponent->bOrientRotationToMovement = true;
}

void UGrapplingComponent::StartGrappleCheck()
{
	//check if we can grapple and we're not already grappling
	if (CanGrapple() && !bIsGrappling)
	{
		//do a line trace to see if the player is aiming at something within grapple range
		FHitResult GrappleHit;

		DoGrappleTrace(GrappleHit, MaxGrappleCheckDistance);

		//check if the line trace hit something
		if (GrappleHit.bBlockingHit)
		{
			//start grappling
			StartGrapple(GrappleHit.GetActor(), GrappleHit);
		}
	}
}

FVector UGrapplingComponent::ProcessGrappleInput(FVector MovementInput)
{
	GrappleInput = MovementInput;

	////print the grapple input to the screen
	//GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("GrappleInput: %s"), *GrappleInput.ToString()));

	////check if the z value of the grapple input is less than 0
	//if (GrappleInput.Z < 0)
	//{
	//	//print downwards to the screen
	//	GEngine->AddOnScreenDebugMessage(2, 5.f, FColor::Green, FString::Printf(TEXT("Downwards")));
	//}
	//else
	//{
	//	//print upwards to the screen
	//	GEngine->AddOnScreenDebugMessage(2, 5.f, FColor::Red, FString::Printf(TEXT("Not Downwards")));
	//}

	//check if the player is grappling, we have valid angle and distance input curves, and the grappling component is valid
	if (bIsGrappling && GrappleMovementAngleInputCurve && GrappleMovementDistanceInputCurve && IsValidLowLevelFast() && !bUseDebugMode)
	{
		//get the dot product of the current grapple direction and the return vector
		const float DotProduct = FVector::DotProduct(GetOwner()->GetActorUpVector(), MovementInput.GetSafeNormal());

		//get the grapple angle movement input curve value
		const float GrappleMovementInputAngleCurveValue = GrappleMovementAngleInputCurve->GetFloatValue(DotProduct);

		//get the grapple distance movement input curve value
		const float GrappleMovementInputDistanceCurveValue = GrappleMovementDistanceInputCurve->GetFloatValue(FMath::Clamp(FVector::Dist(GetOwner()->GetActorLocation(), RopeComponent->GetRopeEnd()) / MaxGrappleDistance, 0, 1));

		//multiply the return vector by the grapple movement input curve value, the grapple distance movement input curve value, and the grapple movement input modifier
		const FVector ReturnVec = MovementInput * GrappleMovementInputAngleCurveValue * GrappleMovementInputDistanceCurveValue * GrappleMovementInputModifier;

		//return the return vector
		return ReturnVec;
	}

	//default to the movement input
	return MovementInput;
}

void UGrapplingComponent::PullPlayer(FVector Vector)
{
	//TODO: check all this code and make sure it's correct

	//check if we're using debug mode
	if (bUseDebugMode)
	{
		//return early
		return;
	}

	//check if we're grappling
	if (bIsGrappling)
	{
		//check if the grapple mode is set to AddToVelocity
		if (GrappleMode == AddToVelocity)
		{
			//add the grapple vector to the character's velocity
			PlayerMovementComponent->Velocity += Vector;
		}
		//check if the grapple mode is set to InterpVelocity
		else if (GrappleMode == InterpVelocity)
		{
			//interpolate the velocity
			PlayerMovementComponent->Velocity = PlayerMovementComponent->ApplySpeedLimit(FMath::VInterpTo(GetOwner()->GetVelocity(), Vector, GetWorld()->GetDeltaSeconds(), GetGrappleInterpStruct().PullAccel), GetWorld()->GetDeltaSeconds());
		}
	}
}

void UGrapplingComponent::DoInterpGrapple(float DeltaTime, FVector& GrappleVelocity, FGrappleInterpStruct GrappleInterpStruct)
{
	//storage for the grapple direction
	const FVector LocGrappleDirection = GetGrappleDirection();

	switch (GrappleInterpStruct.InterpMode)
	{
		case InterpTo:
			//interpolate the velocity
			GrappleVelocity = PlayerMovementComponent->ApplySpeedLimit(FMath::VInterpTo(GetOwner()->GetVelocity(), LocGrappleDirection * GrappleInterpStruct.PullSpeed, DeltaTime, GrappleInterpStruct.PullAccel), DeltaTime);
			break;
		case InterpStep:
			//interpolate the velocity
			GrappleVelocity = PlayerMovementComponent->ApplySpeedLimit(FMath::VInterpTo(GetOwner()->GetVelocity(), LocGrappleDirection * GrappleInterpStruct.PullSpeed, DeltaTime, GrappleInterpStruct.PullAccel), DeltaTime);
			break;
		default /* constant */:
			//interpolate the velocity
			GrappleVelocity = PlayerMovementComponent->ApplySpeedLimit(FMath::VInterpConstantTo(GetOwner()->GetVelocity(),  LocGrappleDirection * GrappleInterpStruct.PullSpeed, DeltaTime, GrappleInterpStruct.PullAccel), DeltaTime);
			break;
	}

	//calculate the grapple dot product
	GrappleDotProduct = GetGrappleDotProduct(GrappleVelocity);

	//calculate the absolute grapple dot product
	AbsoluteGrappleDotProduct = GetAbsoluteGrappleDotProduct(GrappleVelocity);
}

void UGrapplingComponent::DoGrappleTrace(FHitResult& GrappleHit, const float MaxDistance) const
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

	//set the collision shape
	FCollisionShape CollisionShape;
	CollisionShape.ShapeType = CanGrappleCollisionShape;

	//do the line trace
	GetWorld()->SweepSingleByChannel(GrappleHit, CameraLocation + Rotation * 10, End, FQuat::Identity, RopeComponent->CollisionChannel, CollisionShape, GrappleCollisionParams);
}

void UGrapplingComponent::ApplyPullForce(float DeltaTime)
{
	//check if we're using debug mode
	if (bUseDebugMode)
	{
		//return early
		return;
	}

	//storage for the velocity that will be applied from the grapple
	FVector GrappleVelocity;

	//check how we should set the velocity
	// ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
	switch (GetGrappleMode())
	{
		case AddToVelocity:
			//add the grapple vector to the character's velocity
			GrappleVelocity = GetGrappleDirection() * GetPullSpeed() * DeltaTime;

			//check if we have a valid angle velocity curve
			if (GrappleAngleCurve)
			{
				//get the grapple angle velocity curve value
				const float GrappleAngleVelocityCurveValue = GrappleAngleCurve->GetFloatValue(GetGrappleDotProduct(GrappleVelocity.GetSafeNormal()));

				//multiply the grapple velocity by the grapple velocity curve value
				GrappleVelocity *= GrappleAngleVelocityCurveValue;
			}

			//check if we have a valid distance velocity curve
			if (GrappleDistanceCurve)
			{
				//get the grapple distance velocity curve value
				const float GrappleDistanceVelocityCurveValue = GrappleDistanceCurve->GetFloatValue(FMath::Clamp(FVector::Dist(GetOwner()->GetActorLocation(), RopeComponent->GetRopeEnd()) / MaxGrappleDistance, 0, 1));
				 
				//multiply the grapple velocity by the grapple velocity curve value
				GrappleVelocity *= GrappleDistanceVelocityCurveValue;
			}

			////check if we have a valid collision point velocity curve
			//if (GrappleCollisionPointsCurve)
			//{
			//	//get the grapple collision point velocity curve value
			//	const float GrappleCollisionPointVelocityCurveValue = GrappleCollisionPointsCurve->GetFloatValue(RopeComponent->RopePoints.Num() - 1);

			//	//multiply the grapple velocity by the grapple velocity curve value
			//	GrappleVelocity *= GrappleCollisionPointVelocityCurveValue;
			//}

			////check if we have a valid segment length curve
			//if (GrappleLastSegmentLengthCurve)
			//{
			//	//get the grapple segment length pull force curve value
			//	const float GrappleSegmentLengthPullForceCurveValue = GrappleLastSegmentLengthCurve->GetFloatValue(FVector::Dist(RopeComponent->RopePoints[RopeComponent->RopePoints.Num()-2], RopeComponent->GetRopeEnd()) / FVector::Dist(RopeComponent->RopePoints[0], RopeComponent->GetRopeEnd()));

			//	//multiply the grapple velocity by the grapple velocity curve value
			//	GrappleVelocity *= GrappleSegmentLengthPullForceCurveValue;
			//}

			////check if we have a valid grapple velocity curve
			//if (GrappleSpeedAndDirectionForceCurve)
			//{
			//	//get the grapple velocity curve value
			//	const float GrappleVelocityCurveValue = GrappleSpeedAndDirectionForceCurve->GetFloatValue(FVector::DotProduct(GetOwner()->GetVelocity().GetSafeNormal(), GrappleVelocity.GetSafeNormal()) * GetOwner()->GetVelocity().Size() / PlayerMovementComponent->GetMaxSpeed());

			//	//multiply the grapple velocity by the grapple velocity curve value
			//	GrappleVelocity *= GrappleVelocityCurveValue;
			//}

			//calculate the grapple dot product
			GrappleDotProduct = GetGrappleDotProduct(GrappleVelocity);

			//calculate the absolute grapple dot product
			AbsoluteGrappleDotProduct = GetAbsoluteGrappleDotProduct(GrappleVelocity);

			//apply the grapple velocity
			PlayerMovementComponent->Velocity = PlayerMovementComponent->ApplySpeedLimit(PlayerMovementComponent->Velocity + GrappleVelocity, DeltaTime);

		break;
		case InterpVelocity:
			//do the interpolation
			DoInterpGrapple(DeltaTime, PlayerMovementComponent->Velocity, GetGrappleInterpStruct());
		break;
	}
}

void UGrapplingComponent::OnGrappleTargetDestroyed(AActor* DestroyedActor)
{
	//stop grappling
	StopGrapple();
}

FVector UGrapplingComponent::GetGrappleDirection() const
{
	//old code for when the rope component only had two points (instead of a dynamic array of points that can be any length)
	////get the direction from the first rope point to the second rope point
	//return (RopeComponent->GetSecondRopePoint() - GetOwner()->GetActorLocation()).GetSafeNormal();

	////storage for the direction to each rope point individually
	//TArray<FVector> GrappleDirections;

	////for loop to perform the grapple direction checks	
	//for (int Index = 1; Index < GrappleDirectionChecks; ++Index)
	//{
	//	//get the direction from the first rope point to the current rope point
	//	const FVector LocGrappleDirection = (RopeComponent->RopePoints[Index].GetWL() - RopeComponent->RopePoints[0].GetWL()).GetSafeNormal();

	//	//add the direction to the array
	//	GrappleDirections.Add(LocGrappleDirection);
	//}

	////storage for the return vector
	//FVector ReturnVec = FVector::ZeroVector;

	////iterate over the grapple directions array
	//for (const FVector& LocGrappleDirection : GrappleDirections)
	//{
	//	//add the grapple direction to the return vector
	//	ReturnVec += LocGrappleDirection;
	//}

	////normalize the return vector
	//ReturnVec.Normalize();

	////return the return vector
	//return ReturnVec;

	return RopeComponent->GetRopeDirection(GrappleDirectionChecks);
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
		return GetGrappleInterpStruct().PullSpeed;
	}
	//check if we're in the InterpVelocity grapple mode
	if (GrappleMode == InterpVelocity)
	{
		//todo : implement this and replace placeholder code
		//get the difference between the current speed of the owner and the interpolated speed
		return GetOwner()->GetVelocity().Size() - GetGrappleInterpStruct().PullSpeed;
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

bool UGrapplingComponent::CanGrapple() const
{
	//do a line trace to see if the player is aiming at something within grapple range
	FHitResult GrappleHit;

	DoGrappleTrace(GrappleHit, MaxGrappleDistance);

	//check if the line trace didn't hit anything
	if (!GrappleHit.bBlockingHit)
	{
		//return false
		return false;
	}

	//check if the line trace hit something ungrappleable
	if (GrappleHit.GetActor()->ActorHasTag(HiltTags::NoGrappleTag) || GrappleHit.GetComponent()->ComponentHasTag(HiltTags::NoGrappleTag))
	{
		//return false
		return false;
	}

	//return true
	return true;
}

float UGrapplingComponent::GetRemainingGrappleDistance() const
{
	//do a line trace to see if the player is aiming at something within grapple range
	FHitResult GrappleHit;

	DoGrappleTrace(GrappleHit, MaxGrappleCheckDistance);

	//if the line trace hit something, return the distance to the hit
	if (GrappleHit.bBlockingHit)
	{
		//get the distance left until the player can grapple to where they are aiming and check if it's greater than 0
		if (const float GrappleDistanceLeft = FVector::Dist(GetOwner()->GetActorLocation(), GrappleHit.ImpactPoint) - MaxGrappleDistance; GrappleDistanceLeft > 0.f)
		{
			//return the distance left until the player can grapple to where they are aiming
			return GrappleDistanceLeft;
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
