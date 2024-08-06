// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ScoreComponent.h"

#include "Components/PlayerMovementComponent.h"
#include "Components/GrapplingHook/GrapplingComponent.h"
#include "Player/PlayerCharacter.h"

// Sets default values for this component's properties
UScoreComponent::UScoreComponent()
{
	//set the tick function to be enabled
	PrimaryComponentTick.bCanEverTick = true;
}

void UScoreComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//get the owner as a player character
	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
}

void UScoreComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//call the parent tick function
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//check if the score degradation curve is valid and the last score gain time + the score decay delay is less than the current time and that we're not falling and we're walking
	if (ScoreDegradationCurve && LastScoreGainTime + GetCurrentScoreValues().ScoreDecayDelay < GetWorld()->GetTimeSeconds() && bShouldDegrade)
	{
		//get the degradation value from the curve
		const float DegradationValue = ScoreDegradationCurve->GetFloatValue(Score / ScoreValues.Num());

		//degrade the score
		Score -= DegradationValue * DeltaTime;

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
	////store the default score addition value
	//const float DefaultScoreAdditionValue = Score + Value;

	//apply the score addition value
	Score = FMath::Clamp(Score + Value * GetCurrentScoreValues().ScoreGainMultiplier, 0.f, ScoreValues.Num() - 0.01);

	//set the last score gain time
	LastScoreGainTime = GetWorld()->GetTimeSeconds();

	//check if the value is greater than 0
	if (Value > 0)
	{
		//set bShouldDegrade to false
		bShouldDegrade = false;
	}
}

void UScoreComponent::SubtractScore(const float Value)
{
	////store the default score subtraction value
	//const float DefaultScoreSubtractionValue = Score - Value;
	//
	//apply the score subtraction value
	Score = FMath::Clamp(Score - Value * GetCurrentScoreValues().ScoreLossMultiplier, 0.f, ScoreValues.Num() - 0.01);

	//set the last score gain time to -infinity
	LastScoreGainTime = -INFINITY;

	//check if the value is less than 0
	if (Value > 0)
	{
		//set bShouldDegrade to true
		bShouldDegrade = true;
	}
}

void UScoreComponent::ResetScore()
{
	Score = 0;
}

void UScoreComponent::StartDegredationTimer()
{
	//check if we're already degrading
	if (!bShouldDegrade)
	{
		LastScoreGainTime = GetWorld()->GetTimeSeconds();
		bShouldDegrade = true;
	}
}

void UScoreComponent::StopDegredationTimer()
{
	//check if we're degrading
	if (bShouldDegrade)
	{
		bShouldDegrade = false;
	}
}

FScoreValues UScoreComponent::GetCurrentScoreValues() const
{
	//return the score values at the current score
	return ScoreValues[FMath::Min(ScoreValues.Num() - 1, FMath::Floor(Score))];
}

