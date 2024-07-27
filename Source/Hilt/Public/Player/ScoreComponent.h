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

	//the sliding turn rate curve to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Curves")
	UCurveFloat* SlidingTurnRateCurve = nullptr;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HILT_API UScoreComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	//the player's score
	UPROPERTY(BlueprintReadOnly)
	float Score = 0;

	//the max score the player can have
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	float MaxScore = 100;

	//the float curve to use for the player's score degradation over time (1 = 100% of the score, 0 = 0% of the score)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* ScoreDegradationCurve = nullptr;

	//the array of score values to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	TArray<FScoreValues> ScoreValues;

	// Sets default values for this component's properties
	UScoreComponent();

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

	//function to get the current score values
	UFUNCTION(BlueprintCallable)
	FScoreValues GetCurrentScoreValues() const;
		
};
