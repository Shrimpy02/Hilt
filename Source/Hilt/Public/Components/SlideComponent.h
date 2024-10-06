// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SlideComponent.generated.h"

UCLASS()
class USlideComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:

	//events for sliding
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartSlide);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopSlide);

	//start sliding event
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStartSlide OnStartSlide;

	//stop sliding event
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStopSlide OnStopSlide;

	//the float curve to use for processing landing based on the dot product of the player's velocity and the surface normal (-1 = opposite, 1 = aligned)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* SlideLandingDotCurve = nullptr;

	////the float curve to use for the multiplier to apply to the player's speed when slide jumping (0 = min speed, 1 = max speed)
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	//UCurveFloat* SlideJumpSpeedCurve = nullptr;

	////the float curve to use for the multiplier to apply to the player's speed when slide jumping (1 = aligned with velocity, -1 = opposite of velocity)
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	//UCurveFloat* SlideJumpDirectionCurve = nullptr;

	//the speed to add to the player when starting a slide
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSlideStartSpeed = 1000;

	//the slide jump force multiplier
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jumping")
	float SlideJumpForceMultiplier = 1;

	//the slide jump time
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jumping")
	float SlideJumpTime = 0.5;

	//the amount of speed gained from sliding since the last slide start
	float SlideSpeedGained = /*100*/0;

	//the current slide speed (from either landing or starting a slide)
	float CurrentSlideSpeed = 0;

	//storage for the time we started sliding
	float SlideStartTime = 0;

	//whether or not we're slide jumping
	bool bIsSlideJumping = false;

	//the time when the slide fall started
	float SlideFallStartTime = 0;

	//storage for the owner as a player character
	UPROPERTY()
	class APlayerCharacter* PlayerCharacter = nullptr;

	//constructor
	USlideComponent();

	//override(s)
	virtual void BeginPlay() override;

	//function to start sliding
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSlide();

	//function to stop sliding
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSlide();

	//function to get whether or not the player is currently sliding
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsSliding() const;

	//function to get the direction the player is currently sliding
	UFUNCTION(BlueprintCallable, Category = "Movement")
	FVector GetSlideSurfaceDirection();

	//function to get the delta rotation for the player when sliding
	FRotator GetDeltaRotation(float DeltaTime) const;

	//copied from the function of the same name in character movement component
	static float GetAxisDeltaRotation(float InAxisRotationRate, float DeltaTime);
};
