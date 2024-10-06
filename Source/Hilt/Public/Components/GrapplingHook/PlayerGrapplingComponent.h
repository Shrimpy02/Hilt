// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGrapplingComponent.h"
#include "PlayerGrapplingComponent.generated.h"

UCLASS()
class UPlayerGrapplingComponent : public UBaseGrapplingComponent
{
	GENERATED_BODY()
	
public:

	//reference to the player character
	UPROPERTY()
	class APlayerCharacter* PlayerCharacter = nullptr;

	//constructor
	UPlayerGrapplingComponent();

	//override(s)
	virtual void BeginPlay() override;
	virtual float GetPullGrappleModifiers(FVector Direction) const override;
};
