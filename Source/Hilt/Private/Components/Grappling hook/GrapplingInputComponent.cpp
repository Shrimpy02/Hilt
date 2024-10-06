#include "Components/GrapplingHook/GrapplingInputComponent.h"

UGrapplingInputComponent::UGrapplingInputComponent()
{
	bAutoActivate = true;
}

FVector UGrapplingInputComponent::ProcessGrappleInput(FVector MovementInput)
{
	//return the movement input multiplied by the grapple movement input modifiers
	return MovementInput * GetGrappleMovementInputModifiers(MovementInput);
}

float UGrapplingInputComponent::GetGrappleMovementInputModifiers(FVector MovementInput) const
{
	return 1;
}
