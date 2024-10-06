// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrapplingInputComponent.generated.h"

UCLASS()
class UGrapplingInputComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:

	//constructor
	UGrapplingInputComponent();

	//function to process the grapple input
	UFUNCTION(BlueprintCallable)
	virtual FVector ProcessGrappleInput(FVector MovementInput);

	//function to get the input modifiers for the grapple movement
	UFUNCTION(BlueprintCallable)
	virtual float GetGrappleMovementInputModifiers(FVector MovementInput) const;
};
