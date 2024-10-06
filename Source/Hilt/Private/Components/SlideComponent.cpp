#include "Components/SlideComponent.h"

#include "Components/PlayerMovementComponent.h"
#include "Player/PlayerCharacter.h"
#include "Player/ScoreComponent.h"

USlideComponent::USlideComponent()
{
	bAutoActivate = true;
}

void USlideComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//set the player character
	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
}

void USlideComponent::StartSlide()
{
	//set the slide start time
	SlideStartTime = GetWorld()->GetTimeSeconds();

	//check if the velocity is too low
	if (PlayerCharacter->PlayerMovementComponent->Velocity.Size2D() < MinSlideStartSpeed)
	{
		//set the velocity to the minimum slide speed in the direction the player is facing
		PlayerCharacter->PlayerMovementComponent->Velocity = PlayerCharacter->GetActorForwardVector() * MinSlideStartSpeed;
	}

	//set the current slide speed
	CurrentSlideSpeed = PlayerCharacter->PlayerMovementComponent->Velocity.Size();

	//set the player movement component's movement mode to slide
	PlayerCharacter->PlayerMovementComponent->SetMovementMode(MOVE_Custom, ECM_Slide);

	//broadcast the start slide event
	OnStartSlide.Broadcast();
}

void USlideComponent::StopSlide()
{
	//set the player movement component's movement mode to the default land movement mode
	PlayerCharacter->PlayerMovementComponent->SetMovementMode(PlayerCharacter->PlayerMovementComponent->DefaultLandMovementMode);

	//broadcast the stop slide event
	OnStopSlide.Broadcast();
}


bool USlideComponent::IsSliding() const
{
	//check if the player's player movement component is in the custom movement mode and the custom movement mode is slide
	return PlayerCharacter->PlayerMovementComponent->MovementMode == MOVE_Custom && PlayerCharacter->PlayerMovementComponent->CustomMovementMode == ECM_Slide;
}

FVector USlideComponent::GetSlideSurfaceDirection()
{
	//get the normal of the surface we're sliding on
	const FVector SlideNormal = PlayerCharacter->PlayerMovementComponent->CurrentFloor.HitResult.ImpactNormal;

	//get the direction of gravity along the slide surface
	const FVector GravitySurfaceDirection = FVector::VectorPlaneProject(PlayerCharacter->PlayerMovementComponent->GetGravityDirection(), SlideNormal).GetSafeNormal();

	return GravitySurfaceDirection;
}

FRotator USlideComponent::GetDeltaRotation(float DeltaTime) const
{
	//check if we have a valid sliding turn rate curve
	if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().SlidingTurnRateCurve->IsValidLowLevelFast())
	{
		return FRotator(GetAxisDeltaRotation(0, DeltaTime), GetAxisDeltaRotation(PlayerCharacter->ScoreComponent->GetCurrentScoreValues().SlidingTurnRateCurve->GetFloatValue(PlayerCharacter->PlayerMovementComponent->Velocity.Size() / FMath::Max(PlayerCharacter->PlayerMovementComponent->GetMaxSpeed(), PlayerCharacter->PlayerMovementComponent->GetCurrentSpeedLimit())), DeltaTime), GetAxisDeltaRotation(0, DeltaTime));
	}

	return FRotator::ZeroRotator;
}

float USlideComponent::GetAxisDeltaRotation(float InAxisRotationRate, float DeltaTime)
{
	// Values over 360 don't do anything, see FMath::FixedTurn. However we are trying to avoid giant floats from overflowing other calculations.
	return InAxisRotationRate >= 0.f ? FMath::Min(InAxisRotationRate * DeltaTime, 360.f) : 360.f;
}
