#include "Components/GrapplingHook/PlayerGrapplingComponent.h"

#include "Components/PlayerMovementComponent.h"
#include "Components/GrapplingHook/HeadGrapplingComponent.h"
#include "Components/GrapplingHook/PlayerHeadGrapplingComponent.h"
#include "Components/GrapplingHook/RopeComponent.h"
#include "Player/PlayerCharacter.h"
#include "Player/ScoreComponent.h"

UPlayerGrapplingComponent::UPlayerGrapplingComponent()
{
	bAutoActivate = true;
}

void UPlayerGrapplingComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//get the owner as a player character
	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
}

float UPlayerGrapplingComponent::GetPullGrappleModifiers(FVector Direction) const
{
	//storage for the return value
	float ReturnVal = Super::GetPullGrappleModifiers(Direction);

	//check if we have a valid angle curve
	if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleAngleCurve)
	{
		//get the grapple angle curve value
		const float Value = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleAngleCurve->GetFloatValue(FVector::DotProduct(GetOwner()->GetVelocity().GetSafeNormal(), Direction.GetSafeNormal()));

		//multiply the grapple velocity by the grapple curve value
		ReturnVal *= Value;
	}

	//check if we have a valid distance curve
	if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleDistanceCurve)
	{
		//get the grapple distance curve value
		const float Value = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleDistanceCurve->GetFloatValue(FMath::Clamp(FVector::Dist(GetOwner()->GetActorLocation(), PlayerCharacter->RopeComponent->GetRopeEnd()) / PlayerCharacter->HeadGrappleComponent->MaxGrappleDistance, 0, 1));
		 
		//multiply the grapple velocity by the grapple distance curve value
		ReturnVal *= Value;
	}

	//check if we have a valid GrappleVelocityCurve and GrappleVelocityDotProductCurve
	if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleVelocityCurve)
	{
		//get the grapple velocity curve value
		const float VelocityValue = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleVelocityCurve->GetFloatValue(PlayerCharacter->PlayerMovementComponent->Velocity.Size() / PlayerCharacter->PlayerMovementComponent->GetCurrentSpeedLimit());
		
		//multiply the grapple velocity by the grapple velocity curve value
		ReturnVal *= VelocityValue;
	}

	return ReturnVal;
}
