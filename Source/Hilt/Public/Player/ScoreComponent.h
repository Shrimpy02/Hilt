// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScoreComponent.generated.h"

USTRUCT(BlueprintType)
struct FScoreValues
{
	GENERATED_BODY()

	//the grappling hook real speed multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GrappleSpeedMultiplier = 1;

	//the grappling input modifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GrapplingInputModifier = 1;

	//the speed limit modifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedLimitModifier = 1;

	//the gravity scale for the player at this score value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GravityScale = 1;

	//the free time where your score is not degrading after you've gained a bit of score
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScoreDecayDelay = 0;

	//the score gain multiplier for this score value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScoreGainMultiplier = 1;

	//the score loss multiplier for this score value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScoreLossMultiplier = 1;

	//the falling speed multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FallSpeedMultiplier = 1;

	//the hard max limit for the player's speed when falling
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FallSpeedLimit = 6000;

	//the sliding turn rate curve to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* SlidingTurnRateCurve = nullptr;

	//the float curve to use when applying the grapple velocity using the dot product of the character's velocity and the velocity that was added from grappling last frame (-1 = opposite direction, 0 = perpendicular(90 degrees), 1 = same direction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* GrappleAngleCurve = nullptr;

	//the float curve to use when applying the grapple velocity using the rope length divided by the max grapple distance (1 = max distance, 0 = 0 distance, clamped to 0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* GrappleDistanceCurve = nullptr;

	//the float curve used to modify the grapple velocity based on the player's velocity when in addtovelocity mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* GrappleVelocityCurve = nullptr;

	//the float curve to use when applying the grapple wasd movement using the dot product of the character's up vector (so a 90 degree angle off of the the vector pointing to the grappling point) and the velocity that will be added from this input (-1 = opposite direction, 0 = perpendicular(90 degrees), 1 = same direction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* GrappleMovementAngleInputCurve = nullptr;

	//the float curve to use when applying the grapple wasd movement using the rope length divided by the max grapple distance (1 = max distance, 0 = 0 distance, clamped to 0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* GrappleMovementDistanceInputCurve = nullptr;

	//the float curve to modify the grapple wasd movement based on the player's velocity magnitu
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* GrappleMovementSpeedCurve = nullptr;

	//the float curve modify the grapple wasd movement based on the player's velocity direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* GrappleMovementDirectionCurve = nullptr;

	//the curve for the gravity to apply when sliding based on the dot product of the surface normal and the gravity direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* SlideGravityCurve = nullptr;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HILT_API UScoreComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	//the player's score
	UPROPERTY(BlueprintReadOnly)
	float Score = 0;

	//the float curve to use for the player's score degradation over time (1 = 100% of the score, 0 = 0% of the score)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* ScoreDegradationCurve = nullptr;

	//the array of score values to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	TArray<FScoreValues> ScoreValues;

	//last time the player gained score
	UPROPERTY(BlueprintReadOnly)
	float LastScoreGainTime = 0;

	//whether or not the score should degrade
	UPROPERTY(BlueprintReadOnly)
	bool bShouldDegrade = false;

	//reference to the player character associated with this component as a player character
	UPROPERTY()
	class APlayerCharacter* PlayerCharacter = nullptr;

	// Sets default values for this component's properties
	UScoreComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//function to add to the player's score
	UFUNCTION(BlueprintCallable)
	void AddScore(float Value);

	//function to subtract from the player's score
	UFUNCTION(BlueprintCallable)
	void SubtractScore(float Value);

	//function to reset the player's score
	UFUNCTION(BlueprintCallable)
	void ResetScore();

	//start degredation timer
	UFUNCTION(BlueprintCallable)
	void StartDegredationTimer();

	//function to stop the degredation timer
	UFUNCTION(BlueprintCallable)
	void StopDegredationTimer();

	//function to get the current score values
	UFUNCTION(BlueprintCallable)
	FScoreValues GetCurrentScoreValues() const;
		
};
