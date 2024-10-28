#include "Components/PlayerMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/Camera/PlayerCameraComponent.h"
#include "Components/GrapplingHook/RopeComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "InteractableObjects/PylonObjective.h"
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
	else if (Velocity.Size2D() < MinSlideStartSpeed && IsWalking() && !IsFalling() && !IsSliding())
	{
		//set the direction of our velocity to be the forward vector of the player
		Velocity = GetOwner()->GetActorForwardVector() * Velocity.Size();
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

		//set the slide start time
		SlideStartTime = GetWorld()->GetTimeSeconds();

		//bind the slide score banking timer
		GetWorld()->GetTimerManager().SetTimer(SlideScoreBankTimer, this, &UPlayerMovementComponent::BankSlideScore, SlideScoreBankRate, true);
	}

	//set the sliding variable
	bIsSliding = true;

	//stop diving (if we are)
	StopDive();
}

void UPlayerMovementComponent::StopSlide()
{
	//check if we're on the ground
	if (IsWalking())
	{
		//start the score degredation timer
		PlayerPawn->ScoreComponent->StartDegredationTimer();
	}

	//add the pending slide score to the player's score
	PlayerPawn->ScoreComponent->AddScore(PendingSlideScore);

	//set the pending slide score to 0
	PendingSlideScore = 0;

	//set the sliding variable
	bIsSliding = false;

	//reset the slide speed gained
	SlideSpeedGained = 0;

	//call the blueprint event
	OnPlayerStopSlide.Broadcast();

	//unbind the slide score banking timer
	GetWorld()->GetTimerManager().ClearTimer(SlideScoreBankTimer);
}

bool UPlayerMovementComponent::IsSliding() const
{
	return bIsSliding && IsWalking() && !IsFalling() && !PlayerPawn->GrappleComponent->bIsGrappling;
}

void UPlayerMovementComponent::BankSlideScore()
{
	//add the pending slide score to the player's score
	PlayerPawn->ScoreComponent->AddScore(PendingSlideScore);

	//set the pending slide score to 0
	PendingSlideScore = 0;

	//reset the slide speed gained
	SlideSpeedGained = 0;
}

void UPlayerMovementComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//get our owner as a player pawn
	PlayerPawn = Cast<APlayerCharacter>(GetOwner());

	//set the default gravity scale
	DefaultGravityScale = GravityScale;
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

void UPlayerMovementComponent::StartDive()
{
	//check if we're already diving or if we're slide jumping
	if (bIsDiving || IsDiving() || bIsSlideJumping)
	{
		return;
	}

	//set the diving variable
	bIsDiving = true;

	//set the dive start time
	DiveStartTime = GetWorld()->GetTimeSeconds();
}

void UPlayerMovementComponent::StopDive()
{
	//check if we're not diving
	if (!bIsDiving)
	{
		return;
	}

	//set the diving variable
	bIsDiving = false;

	//set the dive stop time
	DiveStopTime = GetWorld()->GetTimeSeconds();

	//set the gravity scale to the default gravity scale
	GravityScale = DefaultGravityScale;
}

bool UPlayerMovementComponent::IsDiving() const
{
	return bIsDiving && IsFalling() && !PlayerPawn->GrappleComponent->bIsGrappling;
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

		//get the direction of gravity along the slide surface
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

		//check if the slide gravity curve is valid
		if (PlayerPawn->ScoreComponent->GetCurrentScoreValues().SlideGravityCurve->IsValidLowLevelFast())
		{
			//add the increase in speed to the current slide speed
			CurrentSlideSpeed += Sign * GravitySurfaceDirection.Size() * PlayerPawn->ScoreComponent->GetCurrentScoreValues().SlideGravityCurve->GetFloatValue(DotProduct) * deltaTime;

			//check if the sign is positive
			if (Sign > 0)
			{
				//add the increase in speed to the slide speed gained
				SlideSpeedGained += Sign * GravitySurfaceDirection.Size() * PlayerPawn->ScoreComponent->GetCurrentScoreValues().SlideGravityCurve->GetFloatValue(DotProduct) * deltaTime;
			}


			//add the slide gravity to the velocity
			Velocity = ApplySpeedLimit(Velocity + GravitySurfaceDirection * PlayerPawn->ScoreComponent->GetCurrentScoreValues().SlideGravityCurve->GetFloatValue(DotProduct) * deltaTime, deltaTime);

			//get the fall speed limit from the score component
			const float FallSpeedLimit = PlayerPawn->ScoreComponent->GetCurrentScoreValues().FallSpeedLimit;

			//clamp the result to the fall speed limit
			Velocity = Velocity.GetClampedToMaxSize(FallSpeedLimit);
		}

		//check if the slide start time + SlideScoreDecayStopDelay is less than the current time
		if (SlideStartTime + SlideScoreDecayStopDelay < GetWorld()->GetTimeSeconds())
		{
			//stop the score degredation timer
			PlayerPawn->ScoreComponent->StopDegredationTimer();
		}

		//check if we have a valid slide score curve
		if (SlideScoreCurve->IsValidLowLevelFast())
		{
			//get the slide score value
			const float SlideScore = SlideScoreCurve->GetFloatValue(SlideSpeedGained / SpeedLimit * PlayerPawn->ScoreComponent->GetCurrentScoreValues().SpeedLimitModifier);

			//update the pending slide score
			PendingSlideScore = SlideScore * PlayerPawn->ScoreComponent->GetCurrentScoreValues().ScoreGainMultiplier;
		}
	}

	//call the parent implementation
	Super::PhysWalking(deltaTime, Iterations);
}

void UPlayerMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
	//check if we're slide falling
	if (bIsSlideFalling)
	{
		//set the velocity to the direction of the camera
		Velocity = PlayerPawn->Camera->GetForwardVector() * Velocity.Size();

		//rotate the character to the velocity direction
		GetCharacterOwner()->SetActorRotation(Velocity.Rotation());

		//check if we should stop slide jumping
		if (GetWorld()->GetTimeSeconds() > SlideFallStartTime + SlideFallStopDelay)
		{
			//set the slide falling variable to false
			bIsSlideFalling = false;
		}
	}

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

	//check if we're slide jumping
	if (bIsSlideJumping)
	{
		//set the velocity to be the direction of the camera
		Velocity = PlayerPawn->Camera->GetForwardVector() * Velocity.Size();

		//rotate the character to the velocity direction
		GetCharacterOwner()->SetActorRotation(Velocity.Rotation());
	}

	//delegate to the parent implementation
	Super::PhysFalling(deltaTime, Iterations);
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

	//section (mostly) copied from the parent implementation start
	FVector Result = InitialVelocity;

	if (DeltaTime > 0.f)
	{
		// Apply gravity.
		Result += Gravity * DeltaTime;

		// Don't exceed terminal velocity.
		float TerminalLimit = FMath::Abs(GetPhysicsVolume()->TerminalVelocity);

		//check if we're diving and we have a valid DiveTerminalVelocityCurve
		if (IsDiving() && DiveTerminalVelocityCurve->IsValidLowLevelFast())
		{
			//get the value from the curve
			const float TerminalVelMultiplier = DiveTerminalVelocityCurve->GetFloatValue(GetWorld()->GetTimeSeconds() - DiveStartTime);

			//multiply the terminal limit by the value
			TerminalLimit *= TerminalVelMultiplier;
		}
		//check if we're diving not diving and we have a valid AfterDiveTerminalVelocityCurve
		else if (!IsDiving() && AfterDiveTerminalVelocityCurve->IsValidLowLevelFast())
		{
			//get the value from the curve
			const float TerminalVelMultiplier = AfterDiveTerminalVelocityCurve->GetFloatValue(GetWorld()->GetTimeSeconds() - DiveStopTime);

			//multiply the terminal limit by the value
			TerminalLimit *= TerminalVelMultiplier;
		}

		if (Result.SizeSquared() > FMath::Square(TerminalLimit))
		{
			const FVector GravityDir = Gravity.GetSafeNormal();
			if ((Result | GravityDir) > TerminalLimit)
			{
				Result = FVector::PointPlaneProject(Result, FVector::ZeroVector, GravityDir) + GravityDir * TerminalLimit;
			}
		}
	}

	//section (mostly) copied from the parent implementation end

	//check if we're applying the speed limit and we're not diving
	if (bIsSpeedLimited && !IsDiving())
	{
		//get the fall speed limit from the score component
		const float FallSpeedLimit = PlayerPawn->ScoreComponent->GetCurrentScoreValues().FallSpeedLimit;

		//add in the after dive terminal velocity curve
		if (AfterDiveTerminalVelocityCurve->IsValidLowLevelFast())
		{
			//get the value from the curve
			const float Value = AfterDiveTerminalVelocityCurve->GetFloatValue(GetWorld()->GetTimeSeconds() - DiveStopTime);

			//clamp the result to the terminal limit multiplied by the value
			return Result.GetClampedToMaxSize(FallSpeedLimit * Value);
		}

		//clamp the result to the fall speed limit
		return Result.GetClampedToMaxSize(FallSpeedLimit);
	}

	return Result;
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

		//return 0 to disable gravity
		return 0;
	}

	//store the result
	float Result =  Super::GetGravityZ();

	//check if we're diving
	if (IsDiving())
	{
		//return the result multiplied by the dive gravity scale multiplier
		return Result *= DiveGravityScaleMultiplier;
	}

	return Result;
}

FVector UPlayerMovementComponent::GetAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration)
{
	//if we're grappling, return the grapple air control (if we're not using normal movement)
	if (PlayerPawn->GrappleComponent->bIsGrappling && !PlayerPawn->GrappleComponent->ShouldUseNormalMovement())
	{
		TickAirControl = PlayerPawn->GrappleComponent->GrappleAirControl;
	}

	//store the result
	FVector Result = Super::GetAirControl(DeltaTime, TickAirControl, FallAcceleration);

	//check if we're diving anc we have a valid DiveWasdCurve
	if (IsDiving() && DiveWasdCurve->IsValidLowLevelFast())
	{
		//get the value from the curve
		const float DiveWasdValue = DiveWasdCurve->GetFloatValue(GetWorld()->GetTimeSeconds() - DiveStartTime);

		//multiply the result by the value
		Result *= DiveWasdValue;
	}

	//return the result
	return Result;
}

void UPlayerMovementComponent::StartFalling(int32 Iterations, float remainingTime, float timeTick, const FVector& Delta, const FVector& subLoc)
{
	//delegate to the parent implementation
	Super::StartFalling(Iterations, remainingTime, timeTick, Delta, subLoc);

	//check if we're sliding
	if (IsSliding())
	{
		//set is slide falling to true
		bIsSlideFalling = true;

		//set the slide fall start time
		SlideFallStartTime = GetWorld()->GetTimeSeconds();

		//stop sliding
		StopSlide();
	}
}

FVector UPlayerMovementComponent::ConsumeInputVector()
{
	//check if we don't have a valid player pawn or we're slide jumping
	if (!PlayerPawn || bIsSlideJumping)
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
	if (IsSliding() || bIsSlideFalling)
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

	//check if we're grappling
	if (PlayerPawn->GrappleComponent->bIsGrappling && !PlayerPawn->GrappleComponent->ShouldUseNormalMovement())
	{
		//return false
		return false;
	}

	//default to the parent implementation
	return Super::IsValidLandingSpot(CapsuleLocation, Hit);
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
	if (IsSliding() && PlayerPawn->ScoreComponent->GetCurrentScoreValues().SlidingTurnRateCurve->IsValidLowLevelFast())
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
			return FMath::Min(GetCurrentSpeedLimit(), PlayerPawn->GrappleComponent->GetMaxSpeed());
		}

		//return the max speed when grappling
		return PlayerPawn->GrappleComponent->GetMaxSpeed();
	}

	//check if we're falling and we're probably not bunny jumping
	if (IsFalling())
	{
		//storage for the max speed to use
		float MaxSpeedToUse = MaxFallSpeed * PlayerPawn->ScoreComponent->GetCurrentScoreValues().FallSpeedMultiplier;

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

		//check if we're diving and we have a valid DiveMaxWasdSpeedCurve curve
		if (IsDiving() && DiveMaxWasdSpeedCurve->IsValidLowLevelFast())
		{
			//get the value
			const float Value = DiveMaxWasdSpeedCurve->GetFloatValue(GetWorld()->GetTimeSeconds() - DiveStartTime);

			//multiply in the value
			MaxSpeedToUse *= Value;
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
		//calcultate the dot product of the velocity and the impact normal
		const float DotProduct = FVector::DotProduct(Velocity.GetSafeNormal(), -Hit.ImpactNormal);

		//invert it by mapping it to the range of 0 to 1
		const float InvertedDotProduct = FMath::GetMappedRangeValueClamped(FVector2D(-1, 1), FVector2D(0, 1), DotProduct);

		//calculate the launch velocity
		FVector UnclampedLaunchVelocity = (Hit.ImpactNormal + Velocity.GetSafeNormal() * InvertedDotProduct).GetSafeNormal() * CollisionLaunchSpeedCurve->GetFloatValue(Velocity.Size() / GetMaxSpeed());

		//check if we're sliding
		if (IsSliding())
		{
			//add the extra force to the launch velocity
			UnclampedLaunchVelocity += FVector::UpVector * SlideCollisionLaunchExtraForce;

			//set the start time of the slide
			SlideStartTime = GetWorld()->GetTimeSeconds();
		}

		//check if we're grappling and not using normal movement
		if (PlayerPawn->GrappleComponent->bIsGrappling && !PlayerPawn->GrappleComponent->ShouldUseNormalMovement())
		{
			//stop grappling
			PlayerPawn->GrappleComponent->StopGrapple();
		}

		//clamp the launch velocity and launch the character
		GetCharacterOwner()->LaunchCharacter(UnclampedLaunchVelocity.GetClampedToSize(MinCollisionLaunchSpeed, MaxCollisionLaunchSpeed), true, true);

		//start the score degredation timer
		PlayerPawn->ScoreComponent->StartDegredationTimer();

		//subtract the score
		PlayerPawn->ScoreComponent->SubtractScore(CollisionScoreLoss);

		//return to prevent further execution
		return;
	}

	//default to the parent implementation
	Super::HandleImpact(Hit, TimeSlice, MoveDelta);
}

void UPlayerMovementComponent::ApplyImpactPhysicsForces(const FHitResult& Impact, const FVector& ImpactAcceleration, const FVector& ImpactVelocity)
{
	//check if we are/just were diving
	if (bIsDiving)
	{
		//stop diving (if we are)
		StopDive();

		//set the velocity back to the impact velocity
		Velocity = ImpactVelocity;

		//start sliding
		StartSlide();

		//check if we have a valid slide landing dot curve
		if (SlideLandingDotCurve->IsValidLowLevelFast())
		{
			//get the dot product of the velocity and the impact normal
			const float DotProduct = FVector::DotProduct(Velocity.GetSafeNormal(), Impact.ImpactNormal);

			//add in the slide landing dot curve to the velocity
			Velocity *= SlideLandingDotCurve->GetFloatValue(DotProduct);

			//add in the slide landing dot curve to the current slide speed
			CurrentSlideSpeed *= SlideLandingDotCurve->GetFloatValue(DotProduct);
		}
	}
	else
	{
		//call the parent implementation
		Super::ApplyImpactPhysicsForces(Impact, ImpactAcceleration, ImpactVelocity);
	}
}

void UPlayerMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	//set might be bunny jumping to true
	bMightBeBunnyJumping = true;

	//set the slide jumping variable to false
	bIsSlideJumping = false;

	//stop grappling (if we're grappling)
	PlayerPawn->GrappleComponent->StopGrapple();

	//call the parent implementation
	Super::ProcessLanded(Hit, remainingTime, Iterations);

	//start the score degredation timer
	PlayerPawn->ScoreComponent->StartDegredationTimer();
}

bool UPlayerMovementComponent::DoJump(bool bReplayingMoves)
{
	//check if we're moving fast enough to do a boosted jump and we're on the ground and that this isn't a double jump
	if (IsSliding())
	{
		//set bIsSlideJumping
		bIsSlideJumping = true;

		//get the direction of the jump
		LastSuperJumpDirection = PlayerPawn->Camera->GetForwardVector();

		//set the movement mode to falling
		SetMovementMode(MOVE_Falling);

		//get the current floor
		FFindFloorResult LocCurrentFloor;
		FindFloor(GetOwner()->GetActorLocation(), LocCurrentFloor, false);

		//get the dot product of the last super jump direction and the impact normal and check if it's less than or equal to 0 (we're jumping into the ground)
		if (const float DotProduct = FVector::DotProduct(LastSuperJumpDirection, LocCurrentFloor.HitResult.ImpactNormal); DotProduct <= 0)
		{
			//use the normalized sum of the last super jump direction and the hit normal
			LastSuperJumpDirection = (LastSuperJumpDirection + CurrentFloor.HitResult.ImpactNormal).GetSafeNormal();
		}

		//variable to store the slide jump force
		float SlideJumpForce = Velocity.Size() * SlideJumpForceMultiplier;

		//check if we have a valid slide jump speed curve
		if (SlideJumpSpeedCurve->IsValidLowLevelFast())
		{
			//get the super jump force
			SlideJumpForce *= SlideJumpSpeedCurve->GetFloatValue(Velocity.Size() / GetMaxSpeed());
		}

		//check if we have a valid slide jump direction curve
		if (SlideJumpDirectionCurve->IsValidLowLevelFast())
		{
			//get the super jump direction
			SlideJumpForce *= SlideJumpDirectionCurve->GetFloatValue(FVector::DotProduct(LastSuperJumpDirection.GetSafeNormal(), Velocity.GetSafeNormal()));
		}

		//launch the character in the direction of the jump
		GetCharacterOwner()->LaunchCharacter(LastSuperJumpDirection * SlideJumpForce, true, true);
		
		//set the bIsSlideJumping variable to true
		bIsSlideJumping = true;

		//set the gravity scale to 0
		GravityScale = 0;

		//throwaway timer handle
		FTimerHandle SlideJumpGravityResetTimer;

		//use timer manager lamba to reset the slide jump variable
		GetWorld()->GetTimerManager().SetTimer(SlideJumpGravityResetTimer, [this]
		{
			bIsSlideJumping = false;
			GravityScale = DefaultGravityScale;
		}, SlideJumpTime, false);

		//call the blueprint event
		OnPlayerSLideJump.Broadcast();

		//stop sliding
		StopSlide();

		//return true
		return true;
	}

	//call the blueprint event for a normal jump
	OnPlayerNormalJump.Broadcast();	

	//default to the parent implementation
	return Super::DoJump(bReplayingMoves);
}
