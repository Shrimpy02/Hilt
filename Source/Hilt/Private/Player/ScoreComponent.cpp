// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ScoreComponent.h"

#include "Components/PlayerMovementComponent.h"
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

	//check if the score degradation curve is valid
	if (ScoreDegradationCurve)
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
	Score = FMath::Clamp(Score + Value, 0.f, ScoreValues.Num() - 1);

	////check if the default score addition value is different from the current score
	//if (DefaultScoreAdditionValue != Score)
	//{
	//	//print the 2 values
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Default Score Addition Value: %f, Score: %f"), DefaultScoreAdditionValue, Score));
	//}
}

void UScoreComponent::SubtractScore(const float Value)
{
	////store the default score subtraction value
	//const float DefaultScoreSubtractionValue = Score - Value;
	//
	//apply the score subtraction value
	Score = FMath::Clamp(Score - Value, 0.f, ScoreValues.Num() - 1);


	////check if the default score subtraction value is different from the current score
	//if (DefaultScoreSubtractionValue != Score)
	//{
	//	//print the 2 values
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Default Score Subtraction Value: %f, Score: %f"), DefaultScoreSubtractionValue, Score));
	//}
}

void UScoreComponent::ResetScore()
{
	Score = 0;
}

FScoreValues UScoreComponent::GetCurrentScoreValues() const
{
	//return the score values at the current score
	return ScoreValues[FMath::Min(ScoreValues.Num() - 1, FMath::Floor(Score))];
}

