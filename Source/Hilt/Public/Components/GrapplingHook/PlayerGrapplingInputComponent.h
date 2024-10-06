// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrapplingInputComponent.h"
#include "PlayerGrapplingInputComponent.generated.h"

UCLASS()
class UPlayerGrapplingInputComponent : public UGrapplingInputComponent
{
	GENERATED_BODY()
	
public:

	//the current grapple input
	UPROPERTY(BlueprintReadOnly)
	FVector GrappleInput = FVector::ZeroVector;

	//the movement input modifier to use when processing the grapple movement input curve
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InputMultiplier = 20;

	//reference to the player character
	UPROPERTY()
	class APlayerCharacter* PlayerCharacter = nullptr;

	//constructor
	UPlayerGrapplingInputComponent();

	//override(s)
	virtual void BeginPlay() override;
	virtual float GetGrappleMovementInputModifiers(FVector MovementInput) const override;
	virtual FVector ProcessGrappleInput(FVector MovementInput) override;

	//whether or not we should use normal movement
	UFUNCTION(BlueprintCallable)
	bool ShouldUseNormalMovement() const;
};
