// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ScoreComponent.h"

// Sets default values for this component's properties
UScoreComponent::UScoreComponent()
{
}

void UScoreComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//call the parent tick function
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//check if the score degradation curve is valid
	if (ScoreDegradationCurve)
	{
		//get the degradation value from the curve
		const float DegradationValue = ScoreDegradationCurve->GetFloatValue(Score / MaxScore);

		//degrade the score
		Score -= MaxScore * DegradationValue * DeltaTime;

		//check if the score is less than 0
		if (Score < 0)
		{
			//set the score to 0
			Score = 0;
		}
	}
}

void UScoreComponent::AddScore(const float Value)
{
	Score += Value;
}

void UScoreComponent::SubtractScore(const float Value)
{
	Score -= Value;
}

FScoreValues UScoreComponent::GetCurrentScoreValues() const
{
	//check if the score values array is valid
	if (ScoreValues.IsValidIndex(FMath::Floor(Score)))
	{
		//return the first score values
		return ScoreValues[FMath::Floor(Score)];
	}

	//return an empty score values struct
	return FScoreValues();
}

