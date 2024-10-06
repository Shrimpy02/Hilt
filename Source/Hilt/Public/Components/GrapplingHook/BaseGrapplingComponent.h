// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGrapplingComponent.generated.h"

UCLASS()
class UBaseGrapplingComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:

	//constructor
	UBaseGrapplingComponent();

	//function to handle the interpolation modes of the grapple
	virtual FVector DoInterpGrapple(float DeltaTime, FVector CurrentVel, FVector TargetPos, float Speed, float Acceleration);

	//function to apply the pull force to the player
	virtual FVector DoPullGrapple(float DeltaTime, FVector CurrentVel, FVector TargetPos, float Speed);

	//function to get the modifiers to use for pull grapple
	virtual float GetPullGrappleModifiers(FVector Direction) const;
};
