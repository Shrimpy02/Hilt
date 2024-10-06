#include "Components/GrapplingHook/PlayerHeadGrapplingComponent.h"

#include "Components/PlayerMovementComponent.h"
#include "Components/GrapplingHook/RopeComponent.h"
#include "NPC/Components/GrappleableComponent.h"
#include "Player/PlayerCharacter.h"
#include "Player/ScoreComponent.h"

UPlayerHeadGrapplingComponent::UPlayerHeadGrapplingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UPlayerHeadGrapplingComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//get the player character
	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
}

void UPlayerHeadGrapplingComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//call the parent implementation
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//check if we're grappling
	if (IsGrappling())
	{
		//check if the player isn't applying input force
		if (PlayerCharacter->PlayerMovementComponent->GetCurrentAcceleration().IsNearlyZero())
		{
			//do the interpolation grapple
			PlayerCharacter->PlayerMovementComponent->Velocity = PlayerCharacter->PlayerGrappleComponent->DoInterpGrapple(DeltaTime, PlayerCharacter->PlayerMovementComponent->Velocity, GetEndLocation(), MaxSpeedInterpMode, AccelerationInterpMode);
		}
		else
		{
			//do the pull grapple
			PlayerCharacter->PlayerMovementComponent->Velocity = PlayerCharacter->PlayerGrappleComponent->DoPullGrapple(DeltaTime, PlayerCharacter->PlayerMovementComponent->Velocity, GetEndLocation(), MaxSpeedPullMode * PullSpeedMultiplier * PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrapplePullModeMultiplier).GetClampedToMaxSize(MaxSpeedPullMode * PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrapplePullModeMultiplier);
		}
	}
}

bool UPlayerHeadGrapplingComponent::IsGrappling() const
{
	//check if the player character is not valid
	if (!PlayerCharacter)
	{
		return false;
	}


	return PlayerCharacter->PlayerMovementComponent->MovementMode == MOVE_Custom && PlayerCharacter->PlayerMovementComponent->CustomMovementMode == ECM_Grapple;
}

void UPlayerHeadGrapplingComponent::StartGrapple(const FHitResult& HitResult)
{
	//set the player movement component's movement mode
	PlayerCharacter->PlayerMovementComponent->SetMovementMode(MOVE_Custom, ECM_Grapple);

	//call the parent implementation
	Super::StartGrapple(HitResult);
}

void UPlayerHeadGrapplingComponent::StopGrapple(const bool CallBlueprintEvent)
{
	//set the movement mode back to falling
	PlayerCharacter->PlayerMovementComponent->SetMovementMode(MOVE_Falling);

	//call the parent implementation
	Super::StopGrapple(CallBlueprintEvent);
}

FHitResult UPlayerHeadGrapplingComponent::GetAimAssistHit(FVector StartLocation, FVector EndLocation) const
{
	//storage for the hit results
	TArray<FHitResult> HitResults;

	//do a sphere multi sweep by channel
	GetWorld()->SweepMultiByChannel(HitResults, StartLocation, EndLocation, FQuat::Identity, GrappleCheckChannel, FCollisionShape::MakeSphere(GrappleSphereRadius), GetCollisionParams());

	for (const FHitResult& GrappleHit : HitResults)
	{
		//get the distance from the line trace start to the hit location
		const float Distance = FVector::Dist(GetOwner()->GetActorLocation(), GrappleHit.ImpactPoint);

		//check if the distance between the trace start and the hit location is less than the distance between the trace start and the player character
		if (Distance - GrappleCheckWiggleRoom < FVector::Dist(GetOwner()->GetActorLocation(), GrappleHit.TraceStart) || GrappleHit.bStartPenetrating)
		{
			continue;
		}

		//return the hit result
		return GrappleHit;
	}

	//return an invalid hit result
	return FHitResult();
}

float UPlayerHeadGrapplingComponent::GetRemainingGrappleDistance() const
{
	//get the distance left until the player can grapple to where they are aiming and check if it's greater than 0
	if (const float GrappleDistanceLeft = FVector::Dist(GetOwner()->GetActorLocation(), RopeComponent->GetRopeEnd()) - MaxGrappleDistance; GrappleDistanceLeft > 0.f)
	{
		//return the distance left until the player can grapple to where they are aiming
		return GrappleDistanceLeft;
	}

	//otherwise return 0
	return 0.f;
}

FHitResult UPlayerHeadGrapplingComponent::GetGrappleHit(float CheckDistance, bool UseAimAssist) const
{
	//storage for camera location and rotation
	FVector CameraLocation;
	FRotator CameraRotation;

	//set the camera location and rotation
	GetOwner()->GetNetOwningPlayer()->GetPlayerController(GetWorld())->GetPlayerViewPoint(CameraLocation, CameraRotation);

	//get the forward vector of the camera rotation
	const FVector Rotation = CameraRotation.Quaternion().GetForwardVector();

	//get the end point of the line trace
	const FVector End = CameraLocation + Rotation * CheckDistance;

	//storage for the hit results
	TArray<FHitResult> HitResults;

	//do the multi line trace
	GetWorld()->LineTraceMultiByChannel(HitResults, CameraLocation, End, GrappleCheckChannel, GetCollisionParams());

	for (const FHitResult& GrappleHit : HitResults)
	{
		//check if the hit location is between the trace start and the owner's location or if the trace started penetrating
		if (const float Distance = FVector::Dist(GetOwner()->GetActorLocation(), GrappleHit.ImpactPoint); Distance - GrappleCheckWiggleRoom < FVector::Dist(GetOwner()->GetActorLocation(), GrappleHit.TraceStart) || GrappleHit.bStartPenetrating)
		{
			//skip this hit result and continue to the next one
			continue;
		}

		//return the hit result
		return GrappleHit;
	}

	//check if we're not using aim assist
	if (!UseAimAssist)
	{
		//return an invalid hit result
		return FHitResult();
	}

	//get the aim assist hit
	return GetAimAssistHit(CameraLocation, End);
}

bool UPlayerHeadGrapplingComponent::PlayerCanGrapple() const
{
	return GetGrappleHit(MaxGrappleDistance, false).IsValidBlockingHit();
}
