#include "Player/PlayerMovementComponent.h"

#include "Player/PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "NPC/Components/GrappleableComponent.h"
#include "Player/GrapplingHook/RopeComponent.h"


UPlayerMovementComponent::UPlayerMovementComponent()
{
	bUseFlatBaseForFloorChecks = true;
	bApplyGravityWhileJumping = false;
	MaxWalkSpeed = 1200.f;
	BrakingFrictionFactor = 0.1f;
	JumpZVelocity = 800.f;
	AirControl = 2.f;
	GravityScale = 4.f;
	FallingLateralFriction = 2.f;
	MaxFlySpeed = 200000.f;
}

void UPlayerMovementComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//get the grappling component
	GrappleComponent = GetOwner()->FindComponentByClass<UGrapplingComponent>();
}

void UPlayerMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//call the parent implementation
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//check if we don't have a valid grappling component
	if (!GrappleComponent->IsValidLowLevelFast())
	{
		return;
	}
}

FVector UPlayerMovementComponent::NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const
{
	//store the result of the parent implementation
	FVector Result;

	//check if we should use terminal velocity
	if (bUseTerminalVelocity == true)
	{
		Result =  Super::NewFallVelocity(InitialVelocity, Gravity, DeltaTime);
	}
	else
	{
		Result = InitialVelocity + Gravity * DeltaTime;
	}

	return Result;
}

void UPlayerMovementComponent::Launch(FVector const& LaunchVel)
{
	Super::Launch(LaunchVel);

	//check if we don't have a valid grappling component
	if (!GrappleComponent->IsValidLowLevelFast())
	{
		return;
	}

	//call the grappling component's stop grapple function
	GrappleComponent->StopGrapple();
}

FVector UPlayerMovementComponent::ConsumeInputVector()
{
	//Store the input vector
	FVector ReturnVec = Super::ConsumeInputVector();

	//check if we don't have a valid grappling component
	if (!GrappleComponent->IsValidLowLevelFast())
	{
		return ReturnVec;
	}

	//check if the input vector is zero
	if (ReturnVec.IsNearlyZero())
	{
		//set the grapple mode to set velocity
		GrappleComponent->SetGrappleMode(InterpVelocity);
	}
	else
	{
		//set the grapple mode to add to velocity
		GrappleComponent->SetGrappleMode(AddToVelocity);
	}

	//check if the player is grappling, we have valid angle and distance input curves, and the grappling component is valid
	if (GrappleComponent->bIsGrappling && GrappleComponent->GrappleMovementAngleInputCurve && GrappleComponent->GrappleMovementDistanceInputCurve && GrappleComponent->IsValidLowLevelFast())
	{
		//get the dot product of the current grapple direction and the return vector
		const float DotProduct = FVector::DotProduct(GetOwner()->GetActorUpVector(), ReturnVec.GetSafeNormal());

		//get the grapple angle movement input curve value
		const float GrappleMovementInputAngleCurveValue = GrappleComponent->GrappleMovementAngleInputCurve->GetFloatValue(DotProduct);

		//get the grapple distance movement input curve value
		const float GrappleMovementInputDistanceCurveValue = GrappleComponent->GrappleMovementDistanceInputCurve->GetFloatValue(FMath::Clamp(FVector::Dist(GetOwner()->GetActorLocation(), GrappleComponent->RopeComponent->GetRopeEnd()) / GrappleComponent->MaxGrappleDistance, 0, 1));

		//multiply the return vector by the grapple movement input curve value, the grapple distance movement input curve value, and the grapple movement input modifier
		ReturnVec *= GrappleMovementInputAngleCurveValue * GrappleMovementInputDistanceCurveValue * GrappleComponent->GrappleMovementInputModifier;
	}

	//return the return vector
	return ReturnVec;
}

bool UPlayerMovementComponent::ShouldRemainVertical() const
{
	//check if we don't have a valid grappling component
	if (!GrappleComponent->IsValidLowLevelFast())
	{
		return Super::ShouldRemainVertical();
	}

	//check if we're grappling
	if (GrappleComponent->bIsGrappling)
	{
		//return false
		return false;
	}

	return Super::ShouldRemainVertical();
}

bool UPlayerMovementComponent::IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const
{
	//if this is a valid landing spot, return true
	if (Super::IsValidLandingSpot(CapsuleLocation, Hit))
	{
		return true;
	}

	//check if the distance from the capsule location to the hit is greater than the capsule half height (to prevent the character from getting stuck on the floor)
	if (const float Distance = FVector::Dist(CapsuleLocation, Hit.ImpactPoint); Distance > GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight())
	{
		return false;
	}

	//check if the surface normal is facing up and the surface is walkable
	if (Hit.ImpactNormal.Z >= GetWalkableFloorZ() && IsWalkable(Hit))
	{
		//return true
		return true;
	}

	return false;
}

float UPlayerMovementComponent::GetGravityZ() const
{
	//check if we don't have a valid grappling component
	if (!GrappleComponent->IsValidLowLevelFast())
	{
		return Super::GetGravityZ();
	}

	//check if the player is grappling
	if (GrappleComponent->bIsGrappling && !GrappleComponent->bApplyGravityWhenGrappling)
	{
		return 0.f;
	}

	return Super::GetGravityZ();
}

float UPlayerMovementComponent::GetMaxSpeed() const
{
	////check if we have a valid max speed curve
	//if (MaxSpeedCurve)
	//{
	//	//get the max speed curve value
	//	return MaxSpeedCurve->GetFloatValue(MaxSpeedDifferenceTime);
	//}

	//check if we have a valid grappling component
	if (GrappleComponent->IsValidLowLevelFast())
	{
		//Check if the player is grappling
		if (GrappleComponent->bIsGrappling)
		{
			//return the max speed when grappling
			return GrappleComponent->GrappleMaxSpeed;
		}
	}

	//check if we're falling
	if (IsFalling())
	{
		//return the max fall speed
		return MaxFallSpeed;
	}

	return Super::GetMaxSpeed();
}

float UPlayerMovementComponent::GetMaxAcceleration() const
{
	//check if we don't have a valid grappling component
	if (!GrappleComponent->IsValidLowLevelFast())
	{
		return Super::GetMaxAcceleration();
	}

	//Check if the player is grappling
	if (GrappleComponent->bIsGrappling)
	{
		//return the max acceleration when grappling
		return GrappleMaxAcceleration;
	}

	return Super::GetMaxAcceleration();
}

void UPlayerMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//check if we're not falling
	if (!IsFalling())
	{
		//delegate to the parent implementation
		Super::HandleImpact(Hit, TimeSlice, MoveDelta);

		return;
	}

	//check if we don't have a physics material or if we have invalid curves
	if (!GetCharacterOwner()->GetCapsuleComponent()->BodyInstance.GetSimplePhysicalMaterial() || !CollisionLaunchSpeedCurve->IsValidLowLevelFast())
	{
		//delegate to the parent implementation
		Super::HandleImpact(Hit, TimeSlice, MoveDelta);

		return;
	}

	//get the bounciness of the physics material
	const float Bounciness = GetCharacterOwner()->GetCapsuleComponent()->BodyInstance.GetSimplePhysicalMaterial()->Restitution;

	//check if the bounciness is less than or equal to 0
	if (Bounciness <= 0)
	{
		//delegate to the parent implementation
		Super::HandleImpact(Hit, TimeSlice, MoveDelta);

		return;
	}

	//calculate the launch velocity
	const FVector UnclampedLaunchVelocity = Hit.ImpactNormal * Bounciness * CollisionLaunchSpeedCurve->GetFloatValue(Velocity.Size() / GetMaxSpeed());

	//clamp the launch velocity and launch the character
	Launch(UnclampedLaunchVelocity.GetClampedToSize(MinCollisionLaunchSpeed, MaxCollisionLaunchSpeed));
}

void UPlayerMovementComponent::ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration)
{
	//check if we have a valid max speed curve
	if (MaxSpeedCurve)
	{
		//add the delta time to the max speed difference time
		MaxSpeedDifferenceTime += DeltaTime;

		//clamp the velocity
		Velocity = Velocity.GetClampedToMaxSize(GetMaxSpeed());
	}

	//check if we're below the max speed threshold
	if (Velocity.Size() < GetMaxSpeed() - MaxSpeedDifference)
	{
		//set the max speed difference time to 0
		MaxSpeedDifferenceTime = 0.f;

		return;
	}

	Super::ApplyVelocityBraking(DeltaTime, Friction, BrakingDeceleration);
}

void UPlayerMovementComponent::OnStartGrapple(AActor* OtherActor, const FHitResult& HitResult)
{
	//check if we're in the walking movement mode
	if (MovementMode == MOVE_Walking)
	{
		//set the movement mode to falling to prevent the player from getting stuck on the floor
		SetMovementMode(MOVE_Flying);
	}
}

void UPlayerMovementComponent::OnStopGrapple()
{
	//empty
}
