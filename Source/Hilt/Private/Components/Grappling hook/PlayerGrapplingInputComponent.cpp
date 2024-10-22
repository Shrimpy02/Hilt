#include "Components/GrapplingHook/PlayerGrapplingInputComponent.h"

#include "Components/PlayerMovementComponent.h"
#include "Components/GrapplingHook/HeadGrapplingComponent.h"
#include "Components/GrapplingHook/PlayerHeadGrapplingComponent.h"
#include "Components/GrapplingHook/RopeComponent.h"
#include "Player/PlayerCharacter.h"
#include "Player/ScoreComponent.h"

UPlayerGrapplingInputComponent::UPlayerGrapplingInputComponent()
 {
	bAutoActivate = true;
}

void UPlayerGrapplingInputComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//get the owner as a player character
	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
}

float UPlayerGrapplingInputComponent::GetGrappleMovementInputModifiers(const FVector MovementInput) const
{
	//storage for the return value
	float ReturnVal = Super::GetGrappleMovementInputModifiers(MovementInput) * InputMultiplier * PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrapplingInputModifier;

	//check if we have a valid score component and it has valid score values
	if (!PlayerCharacter->ScoreComponent->ScoreValues.IsEmpty())
	{
		//return the grapple movement input modifier
		ReturnVal *= PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrapplingInputModifier;
	}

	//check if we have valid angle input curve
	if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementAngleInputCurve)
	{
		//get the dot product of the current grapple direction and the return vector
		const float DotProduct = FVector::DotProduct(PlayerCharacter->GetActorUpVector(),MovementInput.GetSafeNormal());

		//get the grapple angle movement input curve value
		const float Value = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementAngleInputCurve->GetFloatValue(DotProduct);

		//multiply the return vector
		ReturnVal *= Value;
	}

	//check if we have a valid grapple movement distance curve
	if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementDistanceInputCurve)
	{
		//get the grapple distance movement input curve value
		const float Value = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementDistanceInputCurve->GetFloatValue(FMath::Clamp(PlayerCharacter->RopeComponent->GetRopeLength() / PlayerCharacter->HeadGrappleComponent->MaxGrappleDistance, 0, 1));

		//multiply the return 
		ReturnVal *= Value;
	}

	//check if we have a valid GrappleMovementSpeedCurve
	if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementSpeedCurve)
	{
		//get the grapple velocity movement input curve value
		const float Value = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementSpeedCurve->GetFloatValue(PlayerCharacter->PlayerMovementComponent->ApplySpeedLimit(MovementInput).Size() / PlayerCharacter->PlayerMovementComponent->GetCurrentSpeedLimit());

		//multiply the return vector
		ReturnVal *= Value;
	}

	//check if we have a valid GrappleMovementDirectionCurve
	if (PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementDirectionCurve)
	{
		//get the grapple direction movement input curve value
		const float Value = PlayerCharacter->ScoreComponent->GetCurrentScoreValues().GrappleMovementDirectionCurve->GetFloatValue(FVector::DotProduct(MovementInput.GetSafeNormal(), PlayerCharacter->PlayerMovementComponent->Velocity.GetSafeNormal()));

		//multiply the return vector
		ReturnVal *= Value;
	}

	//return the return value
	return ReturnVal;
}

FVector UPlayerGrapplingInputComponent::ProcessGrappleInput(const FVector MovementInput)
{
	//set the grapple input to the movement input
	GrappleInput = MovementInput;

	//check if the player is not grappling or we're using normal movement
	if (!PlayerCharacter->HeadGrappleComponent->IsGrappling() || ShouldUseNormalMovement())
	{
		return MovementInput;
	}

	//return the parent implementation
	return Super::ProcessGrappleInput(MovementInput);
}

bool UPlayerGrapplingInputComponent::ShouldUseNormalMovement() const
{
	//default to false
	return false;
}