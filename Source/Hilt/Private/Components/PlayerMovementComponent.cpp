#include "Components/PlayerMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/DiveComponent.h"
#include "Components/SlideComponent.h"
#include "Components/Camera/PlayerCameraComponent.h"
#include "Components/GrapplingHook/HeadGrapplingComponent.h"
#include "Components/GrapplingHook/PlayerGrapplingInputComponent.h"
#include "Components/GrapplingHook/PlayerHeadGrapplingComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "Player/PlayerCharacter.h"
#include "Player/ScoreComponent.h"


UPlayerMovementComponent::UPlayerMovementComponent()
{
	//set default values
	MaxWalkSpeed = 1200;
	bUseSeparateBrakingFriction = true;
	BrakingFriction = 0.5;
	JumpZVelocity = 1600;
	AirControlBoostMultiplier = 0;
	AirControl = 2;
	GravityScale = 4;
	bApplyGravityWhileJumping = false;
	bOrientRotationToMovement = true;
	BrakingDecelerationWalking = 1536;
}

FVector UPlayerMovementComponent::ApplySpeedLimit(const FVector& InVelocity) const
{
	//return the velocity clamped to the speed limit
	return InVelocity.GetClampedToMaxSize(FMath::Min(GetCurrentSpeedLimit(), GetMaxSpeed()));
}

float UPlayerMovementComponent::GetCurrentSpeedLimit() const
{
	//check if we don't have a valid player pawn
	if (!PlayerCharacter)
	{
		return SpeedLimit;
	}

	//return the speed limit multiplied by the speed limit modifier
	return SpeedLimit * PlayerCharacter->ScoreComponent->GetCurrentScoreValues().SpeedLimitModifier;

}

void UPlayerMovementComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//get our owner as a player pawn
	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
}

void UPlayerMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	//call the parent implementation
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	//check if the new movement mode is custom movement mode
	if (MovementMode == MOVE_Custom)
	{
		//check if the the new custom movement mode is grapple
		if (CustomMovementMode == ECM_Grapple)
		{
			//set bOrientRotationToMovement to false
			bOrientRotationToMovement = false;

			//add 30 to the walkable floor angle
			SetWalkableFloorAngle(GetWalkableFloorAngle() + 30);
		}

		//check if the new custom movement mode is slide
		if (CustomMovementMode == ECM_Slide)
		{
			//set bForceMaxAccel to true
			bForceMaxAccel = true;

			//set the acceleration to the max acceleration
			Acceleration = Velocity.GetSafeNormal() * GetMaxAcceleration();

			//set the velocity
			Velocity = Velocity.GetSafeNormal() * PlayerCharacter->SlideComponent->CurrentSlideSpeed;
		}

		//check if the new custom movement mode is dive
		if (CustomMovementMode == ECM_Dive)
		{
			//multiply in the dive gravity scale multiplier
			GravityScale *= PlayerCharacter->DiveComponent->GravityScaleMultiplier;
		}
	}

	//check if the previous movement mode was custom movement mode
	if (PreviousMovementMode == MOVE_Custom)
	{
		//check if the custom movement mode was grapple
		if (PreviousCustomMode == ECM_Grapple)
		{
			//set bOrientRotationToMovement to true
			bOrientRotationToMovement = true;

			//subtract 30 from the walkable floor angle
			SetWalkableFloorAngle(GetWalkableFloorAngle() - 30);

			//reset the rotation of the owner
			GetOwner()->SetActorRotation(Velocity.Rotation());
		}

		//check if the custom movement mode was slide
		if (PreviousCustomMode == ECM_Slide)
		{
			//set bForceMaxAccel to false
			bForceMaxAccel = false;
		}

		//check if the custom movement mode was dive
		if (PreviousCustomMode == ECM_Dive)
		{
			//divide out the dive gravity scale multiplier
			GravityScale /= PlayerCharacter->DiveComponent->GravityScaleMultiplier;
		}
	}
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

void UPlayerMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	//switch on the custom movement mode
	switch (CustomMovementMode)
	{
		case ECM_Grapple:
		{
			//set the rotation of the character to the grapple direction
			GetOwner()->SetActorRotation((PlayerCharacter->HeadGrappleComponent->GetEndLocation() - PlayerCharacter->GetActorLocation()).GetSafeNormal().Rotation());
			
			//apply the speed limit to the velocity
			Velocity = ApplySpeedLimit(Velocity);

			//storage for the acceleration
			FVector GrappleAcceleration = Acceleration;

			//check if we're enabling gravity while grappling
			if (PlayerCharacter->HeadGrappleComponent->bEnableGravity)
			{
				GrappleAcceleration = GetFallingLateralAcceleration(deltaTime);
				{
					// No acceleration in Z
					const FVector GravityRelativeAcceleration = RotateWorldToGravity(Acceleration);
					GrappleAcceleration = RotateGravityToWorld(FVector(GravityRelativeAcceleration.X, GravityRelativeAcceleration.Y, 0.f));

					// bound acceleration, falling object has minimal ability to impact acceleration
					if (!HasAnimRootMotion() && GravityRelativeAcceleration.SizeSquared2D() > 0.f)
					{
						GrappleAcceleration = GetAirControl(deltaTime, PlayerCharacter->HeadGrappleComponent->AirControl, GrappleAcceleration);
						GrappleAcceleration = GrappleAcceleration.GetClampedToMaxSize(PlayerCharacter->HeadGrappleComponent->MaxAcceleration);
					}
				}


				const FVector GravityRelativeGrappleAcceleration = RotateWorldToGravity(GrappleAcceleration);
				GrappleAcceleration = RotateGravityToWorld(FVector(GravityRelativeGrappleAcceleration.X, GravityRelativeGrappleAcceleration.Y, 0));
			}

			float remainingTime = deltaTime;
			while( remainingTime >= MIN_TICK_TIME && Iterations < MaxSimulationIterations )
			{
				Iterations++;
				float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
				remainingTime -= timeTick;

				const FVector OldVelocityWithRootMotion = Velocity;

				RestorePreAdditiveRootMotionVelocity();

				// Apply input
				if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
				{
					//check if we're enabling gravity while grappling
					if (PlayerCharacter->HeadGrappleComponent->bEnableGravity)
					{
						//function to restore the acceleration
						TGuardValue RestoreAcceleration(Acceleration, GrappleAcceleration);

						//store the old velocity
						const FVector OldVelocity = Velocity;

						//check if we have custom gravity
						if (HasCustomGravity())
						{
							Velocity = FVector::VectorPlaneProject(Velocity, RotateGravityToWorld(FVector::UpVector));
							const FVector GravityRelativeOffset = OldVelocity - Velocity;
							CalcVelocity(timeTick, PlayerCharacter->HeadGrappleComponent->Friction, false, PlayerCharacter->HeadGrappleComponent->MaxBrakingDeceleration);
							Velocity += GravityRelativeOffset;
						}
						else
						{
							Velocity.Z = 0.f;
							CalcVelocity(timeTick, PlayerCharacter->HeadGrappleComponent->Friction, false, PlayerCharacter->HeadGrappleComponent->MaxBrakingDeceleration);
							Velocity.Z = OldVelocity.Z;
						}
					}
					else
					{
						CalcVelocity(timeTick, PlayerCharacter->HeadGrappleComponent->Friction, false, PlayerCharacter->HeadGrappleComponent->MaxBrakingDeceleration);
					}
				}

				//check if we're enabling gravity while grappling
				if (PlayerCharacter->HeadGrappleComponent->bEnableGravity)
				{
					// Compute current gravity
					const FVector Gravity = -GetGravityDirection() * GetGravityZ() / GravityScale * PlayerCharacter->HeadGrappleComponent->GravityScale * PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GravityScale;
					float GravityTime = timeTick;

					// Apply the gravity
					Velocity = NewFallVelocity(Velocity, Gravity, GravityTime);
				}

				ApplyRootMotionToVelocity(timeTick);
				DecayFormerBaseVelocity(timeTick);

				// Compute change in position (using midpoint integration method).
				FVector Adjusted = 0.5f * (OldVelocityWithRootMotion + Velocity) * timeTick;

				// Move
				FHitResult Hit(1.f);
				SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);
				
				float LastMoveTimeSlice = timeTick;
				float subTimeTickRemaining = timeTick * (1.f - Hit.Time);
				
				if ( Hit.bBlockingHit )
				{
					if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
					{
						remainingTime += subTimeTickRemaining;
						ProcessLanded(Hit, remainingTime, Iterations);
						return;
					}

					// Compute impact deflection based on final velocity, not integration step.
					// This allows us to compute a new velocity from the deflected vector, and ensures the full gravity effect is included in the slide result.
					Adjusted = Velocity * timeTick;

					// See if we can convert a normally invalid landing spot (based on the hit result) to a usable one.
					if (!Hit.bStartPenetrating && ShouldCheckForValidLandingSpot(timeTick, Adjusted, Hit))
					{
						const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
						FFindFloorResult FloorResult;
						FindFloor(PawnLocation, FloorResult, false);
						if (FloorResult.IsWalkableFloor() && IsValidLandingSpot(PawnLocation, FloorResult.HitResult))
						{
							remainingTime += subTimeTickRemaining;
							ProcessLanded(FloorResult.HitResult, remainingTime, Iterations);
							return;
						}
					}

					HandleImpact(Hit, LastMoveTimeSlice, Adjusted);
				}
				//check if we're enabling gravity while grappling
				else if (PlayerCharacter->HeadGrappleComponent->bEnableGravity)
				{
					FVector GravityRelativeVelocity = RotateWorldToGravity(Velocity);
					if (GravityRelativeVelocity.SizeSquared2D() <= UE_KINDA_SMALL_NUMBER * 10.f)
					{
						GravityRelativeVelocity.X = 0.f;
						GravityRelativeVelocity.Y = 0.f;
						Velocity = RotateGravityToWorld(GravityRelativeVelocity);
					}
				}
			}
		}
		break;

		case ECM_Slide:
		{
			//update the floor
			FindFloor(PlayerCharacter->GetCapsuleComponent()->GetComponentLocation(),CurrentFloor, false);

			//check if the current floor is not a walkable floor
			if (!CurrentFloor.IsWalkableFloor())
			{
				//start falling
				SetMovementMode(MOVE_Falling);

				return;
			}

			//set the rotation of the character to the velocity direction
			GetOwner()->SetActorRotation(Velocity.Rotation());

			//get the normal of the surface we're sliding on
			const FVector SlideNormal = CurrentFloor.HitResult.ImpactNormal;

			//get the direction of gravity along the slide surface
			const FVector SlideSurfaceDirection = PlayerCharacter->SlideComponent->GetSlideSurfaceDirection();

			//get the dot product of the gravity direction and the slide direction
			const float DotProduct = 1 - FVector::DotProduct(SlideNormal, -GetGravityDirection());

			//get the sign of the dot product of the gravity surface direction and the velocity
			float Sign = FMath::Sign(FVector::DotProduct(Velocity, SlideSurfaceDirection));

			//check if the sign is 0
			if (Sign == 0)
			{
				//set the sign to 1
				Sign = 1;
			}

			//check if the slide gravity curve is valid
			if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().SlideGravityCurve->IsValidLowLevelFast())
			{
				//add the increase in speed to the current slide speed
				PlayerCharacter->SlideComponent->CurrentSlideSpeed += Sign * SlideSurfaceDirection.Size() * PlayerCharacter->ScoreComponent->GetCurrentScoreValues().SlideGravityCurve->GetFloatValue(DotProduct) * deltaTime;

				//check if the sign is positive
				if (Sign > 0)
				{
					//add the increase in speed to the slide speed gained
					PlayerCharacter->SlideComponent->SlideSpeedGained += Sign * SlideSurfaceDirection.Size() * PlayerCharacter->ScoreComponent->GetCurrentScoreValues().SlideGravityCurve->GetFloatValue(DotProduct) * deltaTime;
				}

				//add the slide gravity to the velocity
				Velocity = ApplySpeedLimit(Velocity + SlideSurfaceDirection * PlayerCharacter->ScoreComponent->GetCurrentScoreValues().SlideGravityCurve->GetFloatValue(DotProduct) * deltaTime);

				//get the fall speed limit from the score component
				const float SlideSpeedLimit = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().SlideSpeedLimit;

				//clamp the result to the fall speed limit
				Velocity = Velocity.GetClampedToMaxSize(SlideSpeedLimit);
			}

			//call the walking implementation
			PhysWalking(deltaTime, Iterations);

		}
		break;
		case ECM_Dive:
		{
			//use the phys falling implementation
			PhysFalling(deltaTime, Iterations);

			//update the floor
			FindFloor(PlayerCharacter->GetCapsuleComponent()->GetComponentLocation(),CurrentFloor, false);

			//check if the current floor is valid
			if (CurrentFloor.IsWalkableFloor())
			{
				////set the current slide speed	
				//PlayerCharacter->SlideComponent->CurrentSlideSpeed = Velocity.Size();

				//start sliding
				PlayerCharacter->SlideComponent->StartSlide();

				return;
			}
		}
		break;
		default: ;
	}

	//call the parent implementation
	Super::PhysCustom(deltaTime, Iterations);
}

//bool UPlayerMovementComponent::IsWalkable(const FHitResult& Hit) const
//{
//	//most of this function is copied from the parent implementation
//	if (!Hit.IsValidBlockingHit())
//	{
//		// No hit, or starting in penetration
//		return false;
//	}
//
//	// Never walk up vertical surfaces.
//	const FVector GravityRelativeImpactNormal = RotateWorldToGravity(Hit.ImpactNormal);
//	if (GravityRelativeImpactNormal.Z < UE_KINDA_SMALL_NUMBER)
//	{
//		return false;
//	}
//
//	float TestWalkableZ = GetWalkableFloorZ();
//
//	////check if we have a valid walkable velocity curve and a walkable direction/normals curve
//	//if (WalkabilityVelocityCurve->IsValidLowLevelFast() && WalkabilityDirectionNormalsCurve->IsValidLowLevelFast() && MovementMode != MOVE_Falling)
//	//{
//	//	//get the walkable velocity value
//	//	const float WalkableVelocity = WalkabilityVelocityCurve->GetFloatValue(Velocity.Size() / SpeedLimit);
//
//	//	//get the walkable direction/normals value
//	//	const float WalkableDirectionNormals = WalkabilityDirectionNormalsCurve->GetFloatValue(FVector::DotProduct(Velocity.GetSafeNormal(), GravityRelativeImpactNormal));
//
//	//	//subtrace both values from the test walkable z
//	//	TestWalkableZ -= WalkableVelocity + WalkableDirectionNormals;
//	//}
//
//	// See if this component overrides the walkable floor z.
//	const UPrimitiveComponent* HitComponent = Hit.Component.Get();
//	if (HitComponent)
//	{
//		const FWalkableSlopeOverride& SlopeOverride = HitComponent->GetWalkableSlopeOverride();
//		TestWalkableZ = SlopeOverride.ModifyWalkableFloorZ(TestWalkableZ);
//	}
//
//	// Can't walk on this surface if it is too steep.
//	if (GravityRelativeImpactNormal.Z < TestWalkableZ)
//	{
//		return false;
//	}
//
//	return true;
//}

//todo check if we need to override this function
void UPlayerMovementComponent::PerformMovement(float DeltaTime)
{
	//check if we're sliding
	if (PlayerCharacter->SlideComponent->IsSliding() && PlayerCharacter->CurrentMoveDirection != FVector2D::ZeroVector)
	{
		//get the delta rotation
		const FRotator DeltaRotation = GetDeltaRotation(DeltaTime) * FMath::Sign(PlayerCharacter->CurrentMoveDirection.X);

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
	//all code except the part that gets the terminal velocity multiplier is copied from the parent implementation
	FVector Result = InitialVelocity;

	if (DeltaTime > 0.f)
	{
		// Apply gravity.
		Result += Gravity * DeltaTime;

		// Don't exceed terminal velocity.
		const float TerminalLimit = FMath::Abs(GetPhysicsVolume()->TerminalVelocity) * PlayerCharacter->DiveComponent->GetTerminalVelMultipliers(InitialVelocity, Gravity, DeltaTime);
		if (Result.SizeSquared() > FMath::Square(TerminalLimit))
		{
			const FVector GravityDir = Gravity.GetSafeNormal();
			if ((Result | GravityDir) > TerminalLimit)
			{
				Result = FVector::PointPlaneProject(Result, FVector::ZeroVector, GravityDir) + GravityDir * TerminalLimit;
			}
		}
	}

	return Result;
}

float UPlayerMovementComponent::GetGravityZ() const
{
	if (!PlayerCharacter->IsValidLowLevelFast())
	{
		return Super::GetGravityZ();
	}

	return Super::GetGravityZ();
}

//todo check if we need to override this function
FVector UPlayerMovementComponent::GetAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration)
{
	//store the result
	FVector Result = Super::GetAirControl(DeltaTime, TickAirControl, FallAcceleration);

	//check if we're diving anc we have a valid DiveWasdCurve
	if (PlayerCharacter->DiveComponent->IsDiving() && PlayerCharacter->DiveComponent->WasdCurve->IsValidLowLevelFast())
	{
		//get the value from the curve
		const float DiveWasdValue = PlayerCharacter->DiveComponent->WasdCurve->GetFloatValue(GetWorld()->GetTimeSeconds() - PlayerCharacter->DiveComponent->StartTime);

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
	if (PlayerCharacter->SlideComponent->IsSliding())
	{
		//stop sliding
		PlayerCharacter->SlideComponent->StopSlide();
	}
}

FVector UPlayerMovementComponent::ConsumeInputVector()
{
	//check if we don't have a valid player pawn or we're sliding
	if (!PlayerCharacter || PlayerCharacter->SlideComponent->IsSliding())
	{
		return FVector::ZeroVector;
	}

	//Store the input vector
	const FVector ReturnVec = Super::ConsumeInputVector();

	//check if we're grappling
	if(PlayerCharacter->HeadGrappleComponent->IsGrappling())
	{
		//process the grapple input if there is any
		return PlayerCharacter->PlayerGrapplingInputComponent->ProcessGrappleInput(ReturnVec);	
	}

	//return the parent implementation
	return ReturnVec;
}

float UPlayerMovementComponent::GetMaxBrakingDeceleration() const
{
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
	if (IsWalking() || bMightBeBunnyJumping && IsFalling())
	{
		//set the friction to the value of the sliding friction
		Friction = WalkingBrakingFrictionCurve->GetFloatValue(Velocity.Size() / GetMaxSpeed());
	}

	//call the parent implementation
	Super::ApplyVelocityBraking(DeltaTime, Friction, BrakingDeceleration);
}

void UPlayerMovementComponent::AddImpulse(FVector Impulse, bool bVelocityChange)
{
	//call the parent implementation
	Super::AddImpulse(Impulse, bVelocityChange);

	//broadcast the blueprint event
	OnPlayerImpulse.Broadcast(Impulse, bVelocityChange);
}

//todo delete this after adding the custom slide movement mode implementation
FRotator UPlayerMovementComponent::GetDeltaRotation(float DeltaTime) const
{
	//check if we're sliding
	if (PlayerCharacter->SlideComponent->IsSliding())
	{
		//return the slide delta rotation
		return PlayerCharacter->SlideComponent->GetDeltaRotation(DeltaTime);
	}

	//default to the parent implementation
	return Super::GetDeltaRotation(DeltaTime);
}

float UPlayerMovementComponent::GetMaxSpeed() const
{
	//check if we don't have a valid player pawn
	if (!PlayerCharacter)
	{
		return 0;
	}

	//Check if the player is grappling (and not using normal movement)
	if (PlayerCharacter->HeadGrappleComponent->IsGrappling() && !PlayerCharacter->PlayerGrapplingInputComponent->ShouldUseNormalMovement())
	{
		//return the max speed when grappling or the speed limit, whichever is smaller
		return FMath::Min(GetCurrentSpeedLimit(), PlayerCharacter->HeadGrappleComponent->MaxSpeed * PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleSpeedMultiplier);
	}

	//check if we're falling and we're probably not bunny jumping
	if (IsFalling() && !bMightBeBunnyJumping)
	{
		//storage for the max speed to use
		float MaxSpeedToUse = MaxFallSpeed * PlayerCharacter->ScoreComponent->GetCurrentScoreValues().FallSpeedMultiplier;

		//todo check if we need to add this
		////check if we might be bunny jumping
		//if (bMightBeBunnyJumping)
		//{
		//	MaxSpeedToUse = MaxWalkSpeed;
		//}

		//check if we're diving and we have a valid DiveMaxWasdSpeedCurve curve
		if (PlayerCharacter->DiveComponent->IsDiving() && PlayerCharacter->DiveComponent->MaxSpeedCurve->IsValidLowLevelFast())
		{
			//get the value
			const float Value = PlayerCharacter->DiveComponent->MaxSpeedCurve->GetFloatValue(GetWorld()->GetTimeSeconds() - PlayerCharacter->DiveComponent->StartTime);

			//multiply in the value
			MaxSpeedToUse *= Value;
		}

		//return the max fall speed
		return MaxSpeedToUse;
	}

	//check if we're walking and we're sliding
	if (PlayerCharacter->SlideComponent->IsSliding())
	{
		//return the current slide speed
		return FMath::Min(PlayerCharacter->SlideComponent->CurrentSlideSpeed, GetCurrentSpeedLimit());
	}

	//return the speed limit or the parent implementation, whichever is smaller
	return FMath::Min(GetCurrentSpeedLimit(), Super::GetMaxSpeed());
}

float UPlayerMovementComponent::GetMaxAcceleration() const
{
	//check if we're walking and we have a valid curve
	if ((IsWalking() /*&& MaxWalkingAccelerationCurve*/ && !PlayerCharacter->SlideComponent->IsSliding()) || bMightBeBunnyJumping && IsFalling())
	{
		//return max walking acceleration
		return MaxWalkingAcceleration;
	}

	//check if we're grappling
	if (PlayerCharacter->HeadGrappleComponent->IsGrappling() && !PlayerCharacter->PlayerGrapplingInputComponent->ShouldUseNormalMovement())
	{
		//return the max acceleration
		return PlayerCharacter->HeadGrappleComponent->MaxAcceleration;
	}

	//default to the parent implementation
	return Super::GetMaxAcceleration();
}

void UPlayerMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//check if the dot product of the velocity and the impact normal is less than the negative of the head on collision dot
	if ((FVector::DotProduct(Velocity.GetSafeNormal(), Hit.ImpactNormal) < HeadOnCollisionDot && (IsFalling() && Velocity.Size2D() > CollisionSpeedThreshold) || PlayerCharacter->SlideComponent->IsSliding() || PlayerCharacter->HeadGrappleComponent->IsGrappling()) && !IsWalkable(Hit))
	{
		//calcultate the dot product of the velocity and the impact normal
		const float DotProduct = FVector::DotProduct(Velocity.GetSafeNormal(), -Hit.ImpactNormal);

		//invert it by mapping it to the range of 0 to 1
		const float InvertedDotProduct = FMath::GetMappedRangeValueClamped(FVector2D(-1, 1), FVector2D(0, 1), DotProduct);

		//calculate the launch velocity
		FVector UnclampedLaunchVelocity = (Hit.ImpactNormal + Velocity.GetSafeNormal() * InvertedDotProduct).GetSafeNormal() * CollisionLaunchSpeedCurve->GetFloatValue(Velocity.Size() / GetMaxSpeed());

		//check if we're sliding
		if (PlayerCharacter->SlideComponent->IsSliding())
		{
			//add the extra force to the launch velocity
			UnclampedLaunchVelocity += FVector::UpVector * SlideCollisionLaunchExtraForce;

			//set the start time of the slide
			PlayerCharacter->SlideComponent->SlideStartTime = GetWorld()->GetTimeSeconds();
		}

		//check if we're grappling
		if (PlayerCharacter->HeadGrappleComponent->IsGrappling())
		{
			//stop grappling
			PlayerCharacter->HeadGrappleComponent->StopGrapple();
		}

		//clamp the launch velocity and launch the character
		GetCharacterOwner()->LaunchCharacter(UnclampedLaunchVelocity.GetClampedToSize(MinCollisionLaunchSpeed, MaxCollisionLaunchSpeed), true, true);

		//start the score degredation timer
		PlayerCharacter->ScoreComponent->StartDegredationTimer();

		//subtract the score
		PlayerCharacter->ScoreComponent->SubtractScore(PlayerCharacter->ScoreComponent->CollisionScoreLoss);

		//return to prevent further execution
		return;
	}

	//default to the parent implementation
	Super::HandleImpact(Hit, TimeSlice, MoveDelta);
}

void UPlayerMovementComponent::ApplyImpactPhysicsForces(const FHitResult& Impact, const FVector& ImpactAcceleration, const FVector& ImpactVelocity)
{
	//check if we are/just were diving
	if (PlayerCharacter->DiveComponent->IsDiving())
	{
		//stop diving (if we are)
		PlayerCharacter->DiveComponent->StopDive();

		//set the velocity back to the impact velocity
		Velocity = ImpactVelocity;

		//start sliding
		PlayerCharacter->SlideComponent->StartSlide();

		//check if we have a valid slide landing dot curve
		if (PlayerCharacter->SlideComponent->SlideLandingDotCurve->IsValidLowLevelFast())
		{
			//get the dot product of the velocity and the impact normal
			const float DotProduct = FVector::DotProduct(Velocity.GetSafeNormal(), Impact.ImpactNormal);

			//add in the slide landing dot curve to the velocity
			Velocity *= PlayerCharacter->SlideComponent->SlideLandingDotCurve->GetFloatValue(DotProduct);

			//add in the slide landing dot curve to the current slide speed
			PlayerCharacter->SlideComponent->CurrentSlideSpeed *= PlayerCharacter->SlideComponent->SlideLandingDotCurve->GetFloatValue(DotProduct);
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

	//check if we're grappling
	if (PlayerCharacter->HeadGrappleComponent->IsGrappling())
	{
		//stop grappling
		PlayerCharacter->HeadGrappleComponent->StopGrapple();
	}

	//call the parent implementation
	Super::ProcessLanded(Hit, remainingTime, Iterations);

	//start the score degredation timer
	PlayerCharacter->ScoreComponent->StartDegredationTimer();
}

bool UPlayerMovementComponent::DoJump(bool bReplayingMoves)
{
	////check if we're moving fast enough to do a boosted jump and we're on the ground and that this isn't a double jump
	//if (IsSliding())
	//{
	//	//set bIsSlideJumping
	//	bIsSlideJumping = true;

	//	//get the direction of the jump
	//	LastSuperJumpDirection = PlayerCharacter->Camera->GetForwardVector();

	//	//set the movement mode to falling
	//	SetMovementMode(MOVE_Falling);

	//	//get the current floor
	//	FFindFloorResult LocCurrentFloor;
	//	FindFloor(GetOwner()->GetActorLocation(), LocCurrentFloor, false);

	//	//get the dot product of the last super jump direction and the impact normal and check if it's less than or equal to 0 (we're jumping into the ground)
	//	if (const float DotProduct = FVector::DotProduct(LastSuperJumpDirection, LocCurrentFloor.HitResult.ImpactNormal); DotProduct <= 0)
	//	{
	//		//use the normalized sum of the last super jump direction and the hit normal
	//		LastSuperJumpDirection = (LastSuperJumpDirection + CurrentFloor.HitResult.ImpactNormal).GetSafeNormal();
	//	}

	//	//variable to store the slide jump force
	//	float SlideJumpForce = Velocity.Size() * SlideJumpForceMultiplier;

	//	//check if we have a valid slide jump speed curve
	//	if (SlideJumpSpeedCurve->IsValidLowLevelFast())
	//	{
	//		//get the super jump force
	//		SlideJumpForce *= SlideJumpSpeedCurve->GetFloatValue(Velocity.Size() / GetMaxSpeed());
	//	}

	//	//check if we have a valid slide jump direction curve
	//	if (SlideJumpDirectionCurve->IsValidLowLevelFast())
	//	{
	//		//get the super jump direction
	//		SlideJumpForce *= SlideJumpDirectionCurve->GetFloatValue(FVector::DotProduct(LastSuperJumpDirection.GetSafeNormal(), Velocity.GetSafeNormal()));
	//	}

	//	//launch the character in the direction of the jump
	//	GetCharacterOwner()->LaunchCharacter(LastSuperJumpDirection * SlideJumpForce, true, true);
	//	
	//	//set the bIsSlideJumping variable to true
	//	bIsSlideJumping = true;

	//	//set the gravity scale to 0
	//	GravityScale = 0;

	//	//throwaway timer handle
	//	FTimerHandle SlideJumpGravityResetTimer;

	//	//use timer manager lamba to reset the slide jump variable
	//	GetWorld()->GetTimerManager().SetTimer(SlideJumpGravityResetTimer, [this]
	//	{
	//		bIsSlideJumping = false;
	//		GravityScale = DefaultGravityScale;
	//	}, SlideJumpTime, false);

	//	//call the blueprint event
	//	OnPlayerSLideJump.Broadcast();

	//	//stop sliding
	//	StopSlide();

	//	//return true
	//	return true;
	//}

	//call the blueprint event for a normal jump
	OnPlayerNormalJump.Broadcast();	

	//default to the parent implementation
	return Super::DoJump(bReplayingMoves);
}
