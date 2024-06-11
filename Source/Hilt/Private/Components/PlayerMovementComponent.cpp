#include "Components/PlayerMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "NPC/Components/GrappleableComponent.h"
#include "Player/PlayerCharacter.h"


UPlayerMovementComponent::UPlayerMovementComponent()
{
	//bOrientRotationToMovement = true;
	MaxWalkSpeed = 1200;
	//BrakingFrictionFactor = 0.1;
	JumpZVelocity = 800;
	AirControl = 2;
	GravityScale = 4;
	bApplyGravityWhileJumping = false;
	//FallingLateralFriction = 4;

	BrakingDecelerationWalking = 1024;
}

FVector UPlayerMovementComponent::ApplySpeedLimit(const FVector& InVelocity, const float& InDeltaTime)
{
	//check if we don't have a valid player pawn
	if (!PlayerPawn)
	{
		return InVelocity;
	}

	//storage for the new velocity
	const FVector NewVelocity = InVelocity.GetClampedToMaxSize(FMath::Min(SpeedLimit, GetMaxSpeed()));

	//check if the new velocity is different from the old velocity
	if (NewVelocity != InVelocity)
	{
		//add the difference to the excess speed
		ExcessSpeed += FMath::Abs(NewVelocity.Size() - InVelocity.Size()) * InDeltaTime;

		//clamp the excess speed
		ExcessSpeed = FMath::Clamp(ExcessSpeed, 0.f, MaxExcessSpeed);

		//print on screen debug message of the excess speeds' size to show that we got here
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("Delta Size: %f"), ExcessSpeed));

	}

	//return the velocity clamped to the speed limit
	return NewVelocity;
}

void UPlayerMovementComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//get our owner as a player pawn
	PlayerPawn = Cast<APlayerCharacter>(GetOwner());
}

void UPlayerMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//call the parent implementation
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//apply the degredation rate to the excess speed
	ExcessSpeed -= ExcessSpeedDegredationRate * DeltaTime;

	//clamp the excess speed
	ExcessSpeed = FMath::Clamp(ExcessSpeed, 0.f, MaxExcessSpeed);

	//check if the excess is greater or equal to the stop limit
	if (ExcessSpeed >= ExcessSpeedStopLimit)
	{
		//stop applying the speed limit
		bIsSpeedLimited = false;
	}
	else
	{
		//start applying the speed limit
		bIsSpeedLimited = true;
	}
}

FVector UPlayerMovementComponent::NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const
{
	FVector Result = Super::NewFallVelocity(InitialVelocity, Gravity, DeltaTime);

	//check if jump is providing force
	if (GetCharacterOwner()->JumpForceTimeRemaining > 0 && bLastJumpWasDirectional)
	{
		//add the directional jump glide force to the result
		Result += LastDirectionalJumpDirection * DirectionalJumpGlideForce * DeltaTime;
	}

	//check if we're applying the speed limit
	if (bIsSpeedLimited)
	{
		//return the result clamped to the speed limit
		return Result.GetClampedToMaxSize(SpeedLimit);
	}

	return Result;
}

void UPlayerMovementComponent::Launch(FVector const& LaunchVel)
{
	Super::Launch(LaunchVel.GetClampedToMaxSize(GetMaxSpeed()));
}

FVector UPlayerMovementComponent::ConsumeInputVector()
{
	//check if we don't have a valid player pawn
	if (!PlayerPawn)
	{
		return FVector::ZeroVector;
	}

	//Store the input vector
	const FVector ReturnVec = Super::ConsumeInputVector();

	//check if the input vector is zero
	if (ReturnVec.IsNearlyZero())
	{
		//set the grapple mode to set velocity
		PlayerPawn->GrappleComponent->SetGrappleMode(InterpVelocity);
	}
	else
	{
		//set the grapple mode to add to velocity
		PlayerPawn->GrappleComponent->SetGrappleMode(AddToVelocity);
	}

	//process the grapple input if there is any
	return PlayerPawn->GrappleComponent->ProcessGrappleInput(ReturnVec).GetClampedToMaxSize(GetMaxSpeed());
}

float UPlayerMovementComponent::GetGravityZ() const
{
	if (!PlayerPawn)
	{
		return 0;
	}

	//check if the player is grappling
	if (PlayerPawn->GrappleComponent->bIsGrappling && !PlayerPawn->GrappleComponent->bApplyGravityWhenGrappling)
	{
		return 0.f;
	}

	return Super::GetGravityZ();
}

float UPlayerMovementComponent::GetMaxSpeed() const
{
	//check if we don't have a valid player pawn
	if (!PlayerPawn)
	{
		return 0;
	}

	//Check if the player is grappling
	if (PlayerPawn->GrappleComponent->bIsGrappling)
	{
		//check if we're applying the speed limit
		if (bIsSpeedLimited)
		{
			//return the max speed when grappling or the speed limit, whichever is smaller
			return FMath::Min(SpeedLimit, PlayerPawn->GrappleComponent->GrappleMaxSpeed);
		}

		//return the max speed when grappling
		return PlayerPawn->GrappleComponent->GrappleMaxSpeed;
	}

	//check if we're falling
	if (IsFalling())
	{
		//check if we're applying the speed limit
		if (bIsSpeedLimited)
		{
			//return the max fall speed or the speed limit, whichever is smaller
			return FMath::Min(MaxFallSpeed, SpeedLimit);
		}

		//return the max fall speed
		return MaxFallSpeed;
	}

	//check if we're applying the speed limit
	if (bIsSpeedLimited)
	{
		//return the speed limit or the parent implementation, whichever is smaller
		return FMath::Min(SpeedLimit, Super::GetMaxSpeed());
	}

	return Super::GetMaxSpeed();
}

float UPlayerMovementComponent::GetMaxAcceleration() const
{
	//check if we're moving in the opposite direction of the velocity
	if (IsWalking() && Velocity.Size() > 0 && FVector::DotProduct(Velocity.GetSafeNormal(), GetLastInputVector().GetSafeNormal()) < 0)
	{
		//print on screen debug message to show that we got here
		GEngine->AddOnScreenDebugMessage(69, 5.f, FColor::Red, TEXT("Got Here"));

		//return the max acceleration
		return MaxReverseWalkingAcceleration;
	}

	//check if we're walking and we have a valid curve
	if (IsWalking() && MaxWalkingAccelerationCurve)
	{
		//return the max walking acceleration
		return MaxWalkingAccelerationCurve->GetFloatValue(Velocity.Size() / GetMaxSpeed()) * MaxAcceleration;
	}

	//default to the parent implementation
	return Super::GetMaxAcceleration();
}

void UPlayerMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//check if the surface normal should be considered a floor
	if (Hit.ImpactNormal.Z >= GetWalkableFloorZ())
	{
		//rotate the character to the floor
		GetCharacterOwner()->SetActorRotation(FRotationMatrix::MakeFromX(Hit.ImpactNormal).Rotator());

		//delegate to the parent implementation
		Super::HandleImpact(Hit, TimeSlice, MoveDelta);

		return;
	}

	//get our hitbox
	const UCapsuleComponent* Hitbox = GetCharacterOwner()->GetCapsuleComponent();

	//check if we don't have a physics material or if we have invalid curves or if we're not grappling
	if (!Hitbox->BodyInstance.GetSimplePhysicalMaterial() || !CollisionLaunchSpeedCurve->IsValidLowLevelFast() || !PlayerPawn->GrappleComponent->bIsGrappling)
	{
		//delegate to the parent implementation
		Super::HandleImpact(Hit, TimeSlice, MoveDelta);

		return;
	}

	////check if we're outside the distance to the grapple point to stop grappling
	//if (FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GrappleComponent->RopeComponent->GetRopeEnd()) > PlayerPawn->GrappleComponent->GrappleHitDistance)
	//{
	//	//delegate to the parent implementation
	//	Super::HandleImpact(Hit, TimeSlice, MoveDelta);

	//	return;
	//}

	//get the bounciness of the physics material
	const float Bounciness = Hitbox->BodyInstance.GetSimplePhysicalMaterial()->Restitution;

	//check if the bounciness is less than or equal to 0
	if (Bounciness <= 0)
	{
		//delegate to the parent implementation
		Super::HandleImpact(Hit, TimeSlice, MoveDelta);

		return;
	}

	//check if the dot product of the velocity and the impact normal is less than the negative of the head on collision dot
	if (FVector::DotProduct(Velocity.GetSafeNormal(), Hit.ImpactNormal) < -HeadOnCollisionDot)
	{

		//calculate the launch velocity
		const FVector UnclampedLaunchVelocity = Hit.ImpactNormal * Bounciness * CollisionLaunchSpeedCurve->GetFloatValue(Velocity.Size() / GetMaxSpeed());

		//stop grappling
		PlayerPawn->GrappleComponent->StopGrapple();

		//clamp the launch velocity and launch the character
		GetCharacterOwner()->LaunchCharacter(UnclampedLaunchVelocity.GetClampedToSize(MinCollisionLaunchSpeed, MaxCollisionLaunchSpeed), true, true);

		return;
	}

	//default to the parent implementation
	Super::HandleImpact(Hit, TimeSlice, MoveDelta);
}

void UPlayerMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	//call the parent implementation
	Super::ProcessLanded(Hit, remainingTime, Iterations);

	//stop grappling (if we're grappling)
	PlayerPawn->GrappleComponent->StopGrapple();
}

bool UPlayerMovementComponent::DoJump(bool bReplayingMoves)
{
	//check if we're moving fast enough to do a boosted jump and we're on the ground and that this isn't a double jump
	if (Velocity.Length() >= MinSpeedForBoostedJump && !IsFalling() &&  GetCharacterOwner()->JumpCurrentCount == 0 && !bIsSpeedLimited)
	{
		//get the direction of the jump
		LastDirectionalJumpDirection = GetCharacterOwner()->GetControlRotation().Vector();

		//get the dot product of the camera forward vector and the velocity
		const float DotProduct = FVector::DotProduct(LastDirectionalJumpDirection, CurrentFloor.HitResult.ImpactNormal);

		//set the movement mode to falling
		SetMovementMode(MOVE_Falling);

		//check if the dot product is less than or equal to 0
		if (DotProduct <= 0)
		{
			//set the velocity
			Velocity = ApplySpeedLimit(Velocity + FVector::UpVector * (JumpZVelocity + JumpBoostAmount) + Velocity.GetSafeNormal() * DirectionalJumpForce, DELTA);

			////call the blueprint event
			//OnCorrectedDirectionalJump.Broadcast(LastDirectionalJumpDirection, Velocity.GetSafeNormal());
		}
		else
		{
			//set the velocity
			Velocity = ApplySpeedLimit(Velocity + FVector::UpVector * (JumpZVelocity + JumpBoostAmount) + LastDirectionalJumpDirection * DirectionalJumpForce, DELTA);

			////call the blueprint event
			//OnDirectionalJump.Broadcast(LastDirectionalJumpDirection);
		}

		bLastJumpWasDirectional = true;
	}

	return Super::DoJump(bReplayingMoves);
}
