#include "Components/PlayerMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/Camera/PlayerCameraComponent.h"
#include "Components/GrapplingHook/RopeComponent.h"
#include "NPC/Components/GrappleableComponent.h"
#include "Player/PlayerCharacter.h"
#include "Player/ScoreComponent.h"


UPlayerMovementComponent::UPlayerMovementComponent()
{
	//bOrientRotationToMovement = true;
	MaxWalkSpeed = 1200;
	bUseSeparateBrakingFriction = true;
	BrakingFriction = 0.5;
	JumpZVelocity = 800;
	AirControl = 2;
	GravityScale = 4;
	bApplyGravityWhileJumping = false;
	bOrientRotationToMovement = true;
	//FallingLateralFriction = 4;

	BrakingDecelerationWalking = 1536;
}

FVector UPlayerMovementComponent::ApplySpeedLimit(const FVector& InVelocity, const float& InDeltaTime, bool AddToExcessSpeed) const
{
	//check if we don't have a valid player pawn
	if (!PlayerPawn)
	{
		return InVelocity;
	}

	//storage for the new velocity
	const FVector NewVelocity = InVelocity.GetClampedToMaxSize(FMath::Min(GetCurrentSpeedLimit(), GetMaxSpeed()));

	//check if the new velocity is different from the old velocity
	if (NewVelocity != InVelocity && AddToExcessSpeed)
	{
		//add the difference to the excess speed
		ExcessSpeed += FMath::Abs(NewVelocity.Size() - InVelocity.Size()) * InDeltaTime;

		//clamp the excess speed
		ExcessSpeed = FMath::Clamp(ExcessSpeed, 0.f, MaxExcessSpeed);

	}

	//return the velocity clamped to the speed limit
	return NewVelocity;
}

float UPlayerMovementComponent::GetCurrentSpeedLimit() const
{
	//check if we don't have a valid player pawn
	if (!PlayerPawn)
	{
		return SpeedLimit;
	}

	//return the speed limit multiplied by the speed limit modifier
	return SpeedLimit * PlayerPawn->ScoreComponent->GetCurrentScoreValues().SpeedLimitModifier;

}

void UPlayerMovementComponent::StartSlide()
{
	//check if our velocity is less than the minimum slide start speed
	if (Velocity.Size() < MinSlideStartSpeed && IsWalking() && !IsFalling() && !IsSliding())
	{
		//set the velocity to the minimum slide start speed
		Velocity = GetOwner()->GetActorForwardVector() * MinSlideStartSpeed;
	}
	else if (IsSliding())
	{
		//set the velocity to the current slide speed
		Velocity = Velocity.GetSafeNormal() * CurrentSlideSpeed;
	}

	//set the current slide speed
	CurrentSlideSpeed = Velocity.Size();

	//check if we're not already sliding
	if (!bIsSliding)
	{
		//call the blueprint event
		OnPlayerStartSlide.Broadcast();
	}

	//set the sliding variable
	bIsSliding = true;
}

void UPlayerMovementComponent::StopSlide()
{
	//set the sliding variable
	bIsSliding = false;

	//call the blueprint event
	OnPlayerStopSlide.Broadcast();
}

bool UPlayerMovementComponent::IsSliding() const
{
	return bIsSliding && IsWalking() && !IsFalling() && !PlayerPawn->GrappleComponent->bIsGrappling;
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
}

FVector UPlayerMovementComponent::GetSlideSurfaceDirection()
{
	//get the normal of the surface we're sliding on
	const FVector SlideNormal = CurrentFloor.HitResult.ImpactNormal;

	//get the direction of gravity along the slide surface
	const FVector GravitySurfaceDirection = FVector::VectorPlaneProject(GetGravityDirection(), SlideNormal).GetSafeNormal();

	return GravitySurfaceDirection;
}

void UPlayerMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	//check if we're sliding
	if (IsSliding())
	{
		//rotate the character to the velocity direction
		GetCharacterOwner()->SetActorRotation(Velocity.Rotation());

		//get the normal of the surface we're sliding on
		const FVector SlideNormal = CurrentFloor.HitResult.ImpactNormal;

		const FVector GravitySurfaceDirection = GetSlideSurfaceDirection();

		//get the dot product of the gravity direction and the slide direction
		const float DotProduct = 1 - FVector::DotProduct(SlideNormal, -GetGravityDirection());

		//get the sign of the dot product of the gravity surface direction and the velocity
		float Sign = FMath::Sign(FVector::DotProduct(Velocity, GravitySurfaceDirection));

		//check if the sign is 0
		if (Sign == 0)
		{
			//set the sign to 1
			Sign = 1;
		}

		//add the increase in speed to the current slide speed
		CurrentSlideSpeed += Sign * GravitySurfaceDirection.Size() * SlideGravityCurve->GetFloatValue(DotProduct) * deltaTime;

		//add the slide gravity to the velocity
		Velocity = ApplySpeedLimit(Velocity + GravitySurfaceDirection * SlideGravityCurve->GetFloatValue(DotProduct) * deltaTime, deltaTime);
	}

	//call the parent implementation
	Super::PhysWalking(deltaTime, Iterations);
}

bool UPlayerMovementComponent::IsWalkable(const FHitResult& Hit) const
{
	//most of this function is copied from the parent implementation
	if (!Hit.IsValidBlockingHit())
	{
		// No hit, or starting in penetration
		return false;
	}

	// Never walk up vertical surfaces.
	const FVector GravityRelativeImpactNormal = RotateWorldToGravity(Hit.ImpactNormal);
	if (GravityRelativeImpactNormal.Z < UE_KINDA_SMALL_NUMBER)
	{
		return false;
	}

	float TestWalkableZ = GetWalkableFloorZ();

	////check if we have a valid walkable velocity curve and a walkable direction/normals curve
	//if (WalkabilityVelocityCurve->IsValidLowLevelFast() && WalkabilityDirectionNormalsCurve->IsValidLowLevelFast() && MovementMode != MOVE_Falling)
	//{
	//	//get the walkable velocity value
	//	const float WalkableVelocity = WalkabilityVelocityCurve->GetFloatValue(Velocity.Size() / SpeedLimit);

	//	//get the walkable direction/normals value
	//	const float WalkableDirectionNormals = WalkabilityDirectionNormalsCurve->GetFloatValue(FVector::DotProduct(Velocity.GetSafeNormal(), GravityRelativeImpactNormal));

	//	//subtrace both values from the test walkable z
	//	TestWalkableZ -= WalkableVelocity + WalkableDirectionNormals;
	//}

	// See if this component overrides the walkable floor z.
	const UPrimitiveComponent* HitComponent = Hit.Component.Get();
	if (HitComponent)
	{
		const FWalkableSlopeOverride& SlopeOverride = HitComponent->GetWalkableSlopeOverride();
		TestWalkableZ = SlopeOverride.ModifyWalkableFloorZ(TestWalkableZ);
	}

	// Can't walk on this surface if it is too steep.
	if (GravityRelativeImpactNormal.Z < TestWalkableZ)
	{
		return false;
	}

	return true;
}

void UPlayerMovementComponent::PerformMovement(float DeltaTime)
{
	//check if we're sliding
	if (IsSliding() && PlayerPawn->CurrentMoveDirection != FVector2D::ZeroVector)
	{
		//get the delta rotation
		const FRotator DeltaRotation = GetDeltaRotation(DeltaTime) * FMath::Sign(PlayerPawn->CurrentMoveDirection.X);

		//rotate the velocity by the delta rotation
		Velocity = DeltaRotation.RotateVector(Velocity);
	}

	//call the parent implementation
	Super::PerformMovement(DeltaTime);
}

void UPlayerMovementComponent::HandleWalkingOffLedge(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta)
{
	//call the parent implementation
	Super::HandleWalkingOffLedge(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation, TimeDelta);

	//check if previous floor impact normal and previous floor contact normal are not both zero vectors
	if (PreviousFloorImpactNormal != FVector::ZeroVector && PreviousFloorContactNormal != FVector::ZeroVector)
	{
		//call the blueprint event
		OnPlayerStartFall.Broadcast(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation);
	}
}

FVector UPlayerMovementComponent::NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const
{
	//get the result from the parent implementation
	const FVector Result = Super::NewFallVelocity(InitialVelocity, Gravity, DeltaTime);

	//check if we're applying the speed limit
	if (bIsSpeedLimited)
	{
		////return the result clamped to the speed limit
		//return ApplySpeedLimit(Result, DeltaTime, false);
	}

	return Result;
}

void UPlayerMovementComponent::Launch(FVector const& LaunchVel)
{
	Super::Launch(LaunchVel.GetClampedToMaxSize(GetMaxSpeed()));
}

FVector UPlayerMovementComponent::ConsumeInputVector()
{
	//check if we don't have a valid player pawn or we're perched
	if (!PlayerPawn)
	{
		return FVector::ZeroVector;
	}

	//Store the input vector
	const FVector ReturnVec = Super::ConsumeInputVector();

	//check if we're grappling
	if(PlayerPawn->GrappleComponent->bIsGrappling)
	{
		//process the grapple input if there is any
		return PlayerPawn->GrappleComponent->ProcessGrappleInput(ReturnVec).GetClampedToMaxSize(GetMaxSpeed());	
	}

	//check if we're sliding
	if (IsSliding())
	{
		//return zero vector
		return FVector::ZeroVector;
	}

	//return the parent implementation
	return ReturnVec;
}

float UPlayerMovementComponent::GetMaxBrakingDeceleration() const
{
	//check if we're sliding and walking
	if (IsSliding())
	{
		//return 0
		return 0;
	}

	//check if we're falling and the z velocity is less than 0
	if (IsFalling() && Velocity.Z < 0 && !bMightBeBunnyJumping)
	{
		//return the value of the falling braking friction curve
		return FallingBrakingDecelerationCurve->GetFloatValue(FMath::Abs(Velocity.Z) / GetMaxSpeed());
	}

	//default to the parent implementation
	return Super::GetMaxBrakingDeceleration();
}

void UPlayerMovementComponent::ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration)
{
	//check if we're not sliding and we are walking
	if ((!IsSliding() && IsWalking()) || bMightBeBunnyJumping && IsFalling())
	{
		//set the friction to the value of the sliding friction
		Friction = WalkingBrakingFrictionCurve->GetFloatValue(Velocity.Size() / GetMaxSpeed());
	}
	//check if we're sliding and walking
	else if (IsSliding())
	{
		//set the friction to 0
		Friction = 0;
	}

	//call the parent implementation
	Super::ApplyVelocityBraking(DeltaTime, Friction, BrakingDeceleration);
}

void UPlayerMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	//check if we're sliding and walking and we're not brake sliding
	if (IsSliding())
	{
		//set the friction to the value of the sliding friction
		Friction = SlidingGroundFrictionCurve->GetFloatValue(Velocity.Size() / GetMaxSpeed());
	}

	//check if we're falling and grappling
	if (IsFalling() && PlayerPawn->GrappleComponent->bIsGrappling && !PlayerPawn->GrappleComponent->ShouldUseNormalMovement())
	{
		//set the friction to the value of grapple friction
		Friction = PlayerPawn->GrappleComponent->GrappleFriction;
	}

	//delegate to the parent implementation
	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
}

bool UPlayerMovementComponent::IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const
{
	////check if we're grappling
	//if (PlayerPawn->GrappleComponent->bIsGrappling && Super::IsValidLandingSpot(CapsuleLocation, Hit) == true)
	//{
	//	//check if the surface normal is close to the opposite of the grapple direction
	//	if (const float LocDot = FVector::DotProduct(Hit.ImpactNormal, PlayerPawn->GrappleComponent->GrappleDirection.GetSafeNormal()); LocDot > -0.8)
	//	{
	//		//return false
	//		return false;
	//	}
	//}

	//default to the parent implementation
	return Super::IsValidLandingSpot(CapsuleLocation, Hit);
}

float UPlayerMovementComponent::GetGravityZ() const
{
	//check if we don't have a valid player pawn or we're perched
	if (!PlayerPawn)
	{
		return 0;
	}

	//check if the player is grappling and we're not applying gravity when grappling
	if (PlayerPawn->GrappleComponent->bIsGrappling && !PlayerPawn->GrappleComponent->bApplyGravityWhenGrappling)
	{
		//check if the we have a valid grappleable component
		if (PlayerPawn->GrappleComponent->ShouldUseNormalMovement())
		{
			//use the default implementation
			return Super::GetGravityZ();
		}

		return 0;
	}

	return Super::GetGravityZ();
}

FVector UPlayerMovementComponent::GetAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration)
{
	//if we're grappling, return the grapple air control (if we're not using normal movement)
	if (PlayerPawn->GrappleComponent->bIsGrappling && !PlayerPawn->GrappleComponent->ShouldUseNormalMovement())
	{
		TickAirControl = PlayerPawn->GrappleComponent->GrappleAirControl;
	}

	return Super::GetAirControl(DeltaTime, TickAirControl, FallAcceleration);
}

void UPlayerMovementComponent::StartFalling(int32 Iterations, float remainingTime, float timeTick, const FVector& Delta, const FVector& subLoc)
{
	//delegate to the parent implementation
	Super::StartFalling(Iterations, remainingTime, timeTick, Delta, subLoc);
}

void UPlayerMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
	//check if we might be bunny jumping
	if (bMightBeBunnyJumping)
	{
		//storage for the line trace
		FHitResult Hit;

		//do a line trace straight down from the player with the bunny jump trace distance
		GetWorld()->LineTraceSingleByChannel(Hit, GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() - FVector::UpVector * AvoidBunnyJumpTraceDistance, ECC_Visibility);

		//check if it wasn't a valid blocking hit and we didn't start penetrating
		if (!Hit.IsValidBlockingHit() && !Hit.bStartPenetrating)
		{
			//set might be bunny jumping to false
			bMightBeBunnyJumping = false;
		}
	}

	//delegate to the parent implementation
	Super::PhysFalling(deltaTime, Iterations);
}

void UPlayerMovementComponent::AddImpulse(FVector Impulse, bool bVelocityChange)
{
	//call the parent implementation
	Super::AddImpulse(Impulse, bVelocityChange);

	//broadcast the blueprint event
	OnPlayerImpulse.Broadcast(Impulse, bVelocityChange);
}

//copied from the function in the parent class
float UPlayerMovementComponent::GetAxisDeltaRotation(float InAxisRotationRate, float DeltaTime)
{
	// Values over 360 don't do anything, see FMath::FixedTurn. However we are trying to avoid giant floats from overflowing other calculations.
	return InAxisRotationRate >= 0.f ? FMath::Min(InAxisRotationRate * DeltaTime, 360.f) : 360.f;
}

FRotator UPlayerMovementComponent::GetDeltaRotation(float DeltaTime) const
{
	//check if we're sliding and walking
	if (IsSliding())
	{
		return FRotator(GetAxisDeltaRotation(0, DeltaTime), GetAxisDeltaRotation(PlayerPawn->ScoreComponent->GetCurrentScoreValues().SlidingTurnRateCurve->GetFloatValue(Velocity.Size() / FMath::Max(GetMaxSpeed(), GetCurrentSpeedLimit())), DeltaTime), GetAxisDeltaRotation(0, DeltaTime));
	}

	//default to the parent implementation
	return Super::GetDeltaRotation(DeltaTime);
}

float UPlayerMovementComponent::GetMaxSpeed() const
{
	//check if we don't have a valid player pawn
	if (!PlayerPawn)
	{
		return 0;
	}

	//Check if the player is grappling (and not using normal movement)
	if (PlayerPawn->GrappleComponent->bIsGrappling && !PlayerPawn->GrappleComponent->ShouldUseNormalMovement())
	{
		//check if we're applying the speed limit
		if (bIsSpeedLimited)
		{
			//return the max speed when grappling or the speed limit, whichever is smaller
			return FMath::Min(GetCurrentSpeedLimit(), PlayerPawn->GrappleComponent->GrappleMaxSpeed);
		}

		//return the max speed when grappling
		return PlayerPawn->GrappleComponent->GrappleMaxSpeed;
	}

	//check if we're falling and we're probably not bunny jumping
	if (IsFalling())
	{
		//storage for the max speed to use
		float MaxSpeedToUse = MaxFallSpeed;

		//check if we might be bunny jumping
		if (bMightBeBunnyJumping)
		{
			MaxSpeedToUse = MaxWalkSpeed;
		}

		//check if we're applying the speed limit
		if (bIsSpeedLimited)
		{
			//return the max fall speed or the speed limit, whichever is smaller
			return FMath::Min(MaxSpeedToUse, GetCurrentSpeedLimit());
		}

		//return the max fall speed
		return MaxSpeedToUse;
	}

	//check if we're walking and we're sliding
	if (IsSliding())
	{
		//return the current slide speed
		return CurrentSlideSpeed;
	}

	//check if we're applying the speed limit
	if (bIsSpeedLimited)
	{
		//return the speed limit or the parent implementation, whichever is smaller
		return FMath::Min(GetCurrentSpeedLimit(), Super::GetMaxSpeed());
	}

	return Super::GetMaxSpeed();
}

float UPlayerMovementComponent::GetMaxAcceleration() const
{
	//check if we're walking and we have a valid curve
	if ((IsWalking() /*&& MaxWalkingAccelerationCurve*/ && !IsSliding()) || bMightBeBunnyJumping && IsFalling())
	{
		//return max walking acceleration
		return MaxWalkingAcceleration;
	}

	//default to the parent implementation
	return Super::GetMaxAcceleration();
}

void UPlayerMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//todo add a bit of launch upwards when sliding

	//check if the surface normal should be considered a floor
	if (IsWalkable(Hit)) 
	{
		//rotate the character to the floor
		GetCharacterOwner()->SetActorRotation(FRotationMatrix::MakeFromX(Hit.Normal).Rotator());

		//set the movement mode to walking
		SetMovementMode(MOVE_Walking);


		//check if we're not using normal movement
		if (PlayerPawn->GrappleComponent->bIsGrappling && !PlayerPawn->GrappleComponent->ShouldUseNormalMovement())
		{
			//stop grappling
			PlayerPawn->GrappleComponent->StopGrapple();
		}

		return;
	}

	////get our hitbox
	//const UCapsuleComponent* Hitbox = GetCharacterOwner()->GetCapsuleComponent();

	////check if we don't have a physics material or if we have invalid curves or if we're not grappling
	//if (!Hitbox->BodyInstance.GetSimplePhysicalMaterial() || !CollisionLaunchSpeedCurve->IsValidLowLevelFast() || !(PlayerPawn->GrappleComponent->bIsGrappling || IsSliding()) || (Velocity2D < CollisionSpeedThreshold && IsFalling()))
	//{
	//	//delegate to the parent implementation
	//	Super::HandleImpact(Hit, TimeSlice, MoveDelta);

	//	return;
	//}

	////get the bounciness of the physics material
	//const float Bounciness = Hitbox->BodyInstance.GetSimplePhysicalMaterial()->Restitution;

	////check if the bounciness is less than or equal to 0
	//if (Bounciness <= 0)
	//{
	//	//delegate to the parent implementation
	//	Super::HandleImpact(Hit, TimeSlice, MoveDelta);

	//	return;
	//}

	//check if the dot product of the velocity and the impact normal is less than the negative of the head on collision dot
	if (FVector::DotProduct(Velocity.GetSafeNormal(), Hit.ImpactNormal) < HeadOnCollisionDot && (IsFalling() && Velocity.Size2D() > CollisionSpeedThreshold) || IsSliding() || (PlayerPawn->GrappleComponent->bIsGrappling && !PlayerPawn->GrappleComponent->ShouldUseNormalMovement()))
	{
		//calculate the launch velocity
		FVector UnclampedLaunchVelocity = Hit.ImpactNormal * CollisionLaunchSpeedCurve->GetFloatValue(Velocity.Size() / GetMaxSpeed());

		//check if we're sliding
		if (IsSliding())
		{
			//add the extra force to the launch velocity
			UnclampedLaunchVelocity += FVector::UpVector * SlideCollisionLaunchExtraForce;
		}

		//check if we're grappling and not using normal movement
		if (PlayerPawn->GrappleComponent->bIsGrappling && !PlayerPawn->GrappleComponent->ShouldUseNormalMovement())
		{
			//stop grappling
			PlayerPawn->GrappleComponent->StopGrapple();
		}

		//clamp the launch velocity and launch the character
		GetCharacterOwner()->LaunchCharacter(UnclampedLaunchVelocity.GetClampedToSize(MinCollisionLaunchSpeed, MaxCollisionLaunchSpeed), true, true);

		//reset the player's score
		PlayerPawn->ScoreComponent->ResetScore();

		//return to prevent further execution
		return;
	}

	//default to the parent implementation
	Super::HandleImpact(Hit, TimeSlice, MoveDelta);
}

void UPlayerMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	//set might be bunny jumping to true
	bMightBeBunnyJumping = true;

	//call the parent implementation
	Super::ProcessLanded(Hit, remainingTime, Iterations);

	//stop grappling (if we're grappling)
	PlayerPawn->GrappleComponent->StopGrapple();

	//check if we're sliding
	if (IsSliding())
	{
		//set the current slide speed
		CurrentSlideSpeed = Velocity.Size();

		//check if the velocity is less than the minimum slide start speed
		if (Velocity.Size() < MinSlideStartSpeed)
		{
			//set the velocity to the minimum slide start speed
			Velocity = GetOwner()->GetActorForwardVector() * MinSlideStartSpeed;
		}
	}
}

bool UPlayerMovementComponent::DoJump(bool bReplayingMoves)
{
	//check if we're moving fast enough to do a boosted jump and we're on the ground and that this isn't a double jump
	if (IsSliding())
	{
		//get the direction of the jump
		LastSuperJumpDirection = GetCharacterOwner()->GetControlRotation().Vector();

		//get the dot product of the camera forward vector and the velocity
		const float DotProduct = FVector::DotProduct(LastSuperJumpDirection, CurrentFloor.HitResult.ImpactNormal);

		//set the movement mode to falling
		SetMovementMode(MOVE_Falling);

		//check if the dot product is less than or equal to 0
		if (DotProduct <= 0)
		{
			//set the velocity
			Velocity += FVector::UpVector * JumpBoostAmount + ApplySpeedLimit(Velocity.GetSafeNormal() * SuperJumpForce, DELTA);
		}
		else
		{
			//set the velocity
			Velocity += FVector::UpVector * JumpBoostAmount + ApplySpeedLimit(LastSuperJumpDirection * SuperJumpForce, DELTA);
		}

		//call the blueprint event
		OnPlayerSuperJump.Broadcast();
	}
	else
	{
		//call the blueprint event
		OnPlayerNormalJump.Broadcast();	
	}

	//default to the parent implementation
	return Super::DoJump(bReplayingMoves);
}
