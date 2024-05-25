#include "Player/PlayerMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "NPC/Components/GrappleableComponent.h"
#include "Player/PlayerCharacter.h"
#include "Player/GrapplingHook/RopeComponent.h"


UPlayerMovementComponent::UPlayerMovementComponent()
{
}

void UPlayerMovementComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//get our owner as a player pawn
	PlayerPawn = Cast<APlayerCharacter>(GetOwner());
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

	////add the input vector as a force to the player pawn
	//PlayerPawn->SphereComponent->AddForce(ReturnVec * WasdMovementForce, NAME_None, true);
	
	//print that we're consuming the input vector and the return vector
	UE_LOG(LogTemp, Log, TEXT("Consuming input vector: %s"), *ReturnVec.ToString());

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
	return PlayerPawn->GrappleComponent->ProcessGrappleInput(ReturnVec);
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
		//return the max speed when grappling
		return PlayerPawn->GrappleComponent->GrappleMaxSpeed;
	}

	//check if we're falling
	if (IsFalling())
	{
		//return the max fall speed
		return MaxFallSpeed;
	}

	return Super::GetMaxSpeed();
}

void UPlayerMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
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
