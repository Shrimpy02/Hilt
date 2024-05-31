#include "Components/PlayerMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "NPC/Components/GrappleableComponent.h"
#include "Player/PlayerCharacter.h"


UPlayerMovementComponent::UPlayerMovementComponent()
{
	//bOrientRotationToMovement = true;
	//bApplyGravityWhileJumping = false;
	//MaxWalkSpeed = 1200.f;
	//BrakingFrictionFactor = 0.1f;
	//JumpZVelocity = 800.f;
	//AirControl = 2.f;
	//GravityScale = 4.f;
	//FallingLateralFriction = 4.f;
}

void UPlayerMovementComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//get our owner as a player pawn
	PlayerPawn = Cast<APlayerCharacter>(GetOwner());
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

	//calculate the launch velocity
	const FVector UnclampedLaunchVelocity = Hit.ImpactNormal * Bounciness * CollisionLaunchSpeedCurve->GetFloatValue(Velocity.Size() / GetMaxSpeed());

	//stop grappling
	PlayerPawn->GrappleComponent->StopGrapple();

	//clamp the launch velocity and launch the character
	GetCharacterOwner()->LaunchCharacter(UnclampedLaunchVelocity.GetClampedToSize(MinCollisionLaunchSpeed, MaxCollisionLaunchSpeed), true, true);

	Super::HandleImpact(Hit, TimeSlice, MoveDelta);
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
			Velocity += FVector::UpVector * (JumpZVelocity + JumpBoostAmount) + Velocity.GetSafeNormal() * DirectionalJumpForce;

			////call the blueprint event
			//OnCorrectedDirectionalJump.Broadcast(LastDirectionalJumpDirection, Velocity.GetSafeNormal());
		}
		else
		{
			//set the velocity
			Velocity += FVector::UpVector * (JumpZVelocity + JumpBoostAmount) + LastDirectionalJumpDirection * DirectionalJumpForce;

			////call the blueprint event
			//OnDirectionalJump.Broadcast(LastDirectionalJumpDirection);
		}

		bLastJumpWasDirectional = true;
	}

	return Super::DoJump(bReplayingMoves);
}
