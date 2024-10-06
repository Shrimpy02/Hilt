// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ScoreComponent.h"

#include "Components/PlayerMovementComponent.h"
#include "Components/SlideComponent.h"
#include "Components/GrapplingHook/PlayerGrapplingComponent.h"
#include "Components/GrapplingHook/PlayerHeadGrapplingComponent.h"
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

	//bind the stop grapple event
	PlayerCharacter->HeadGrappleComponent->OnStopGrapple.AddDynamic(this, &UScoreComponent::OnStopGrapple);

	//bind the start sliding event
	PlayerCharacter->SlideComponent->OnStartSlide.AddDynamic(this, &UScoreComponent::OnStartSliding);

	//bind the stop sliding event
	PlayerCharacter->SlideComponent->OnStopSlide.AddDynamic(this, &UScoreComponent::OnStopSliding);
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

	if (PlayerCharacter->HeadGrappleComponent->IsGrappling())
	{
		//update the grappling score
		GrapplingScoreUpdate();
	}
	else if (PlayerCharacter->SlideComponent->IsSliding())
	{
		//update the sliding score
		SlidingScoreUpdate();
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
	//check if we're not already degrading
	if (!bShouldDegrade)
	{
		//set the last score gain time
		LastScoreGainTime = GetWorld()->GetTimeSeconds();

		//set the degrading flag
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

float UScoreComponent::GetTotalPendingScore() const
{
	return PendingScore;
}

void UScoreComponent::GrapplingScoreUpdate()
{
	//check if the grapplescorecurve is valid
	if (GrappleScoreCurve)
	{
		//get the grapple score curve value
		const float Value = GrappleScoreCurve->GetFloatValue(GetWorld()->GetTimeSeconds() - PlayerCharacter->HeadGrappleComponent->GrappleStartTime);

		//set the pending score
		PendingScore = Value * GetCurrentScoreValues().ScoreGainMultiplier;
	}

	//check if the grapple start time + GrappleScoreDecayStopDelay is less than the current time
	if (PlayerCharacter->HeadGrappleComponent->GrappleStartTime + GrappleScoreDecayStopDelay < GetWorld()->GetTimeSeconds())
	{
		//stop the score degredation timer
		StopDegredationTimer();
	}
}

void UScoreComponent::OnStopGrapple()
{
	//check if the grapplescorecurve is valid
	if (GrappleScoreCurve)
	{
		//get the grapple score curve value
		const float Value = GrappleScoreCurve->GetFloatValue(GetWorld()->GetTimeSeconds() - PlayerCharacter->HeadGrappleComponent->GrappleStartTime);

		//set the pending score to 0
		PendingScore = 0;

		//add the grapple score curve value to the player's score
		AddScore(Value);
	}
}

void UScoreComponent::SlidingScoreUpdate()
{
	//check if the slide start time + SlideScoreDecayStopDelay is less than the current time
	if (PlayerCharacter->SlideComponent->SlideStartTime + SlideScoreDecayStopDelay < GetWorld()->GetTimeSeconds())
	{
		//stop the score degredation timer
		StopDegredationTimer();
	}

	//check if we have a valid slide score curve
	if (SlideScoreCurve->IsValidLowLevelFast())
	{
		//get the slide score value
		const float SlideScore = SlideScoreCurve->GetFloatValue(PlayerCharacter->SlideComponent->SlideSpeedGained / PlayerCharacter->PlayerMovementComponent->SpeedLimit * GetCurrentScoreValues().SpeedLimitModifier);

		//update the pending slide score
		PendingScore = SlideScore * GetCurrentScoreValues().ScoreGainMultiplier;
	}
}

void UScoreComponent::OnStartSliding()
{
	//bind the slide score banking timer
	GetWorld()->GetTimerManager().SetTimer(SlideScoreBankTimer, this, &UScoreComponent::BankSlideScore, 0.1, true);
}

void UScoreComponent::OnStopSliding()
{
	//add the pending slide score to the player's score
	AddScore(PendingScore);

	//set the pending slide score to 0
	PendingScore = 0;

	//unbind the slide score banking timer
	GetWorld()->GetTimerManager().ClearTimer(SlideScoreBankTimer);

	//check if the player is on the ground
	if (PlayerCharacter->PlayerMovementComponent->IsWalking())
	{
		//start the score degredation timer
		StartDegredationTimer();
	}
}

void UScoreComponent::BankSlideScore()
{
	//add the pending slide score to the player's score
	AddScore(PendingScore);

	//set the pending slide score to 0
	PendingScore = 0;

	//set the slide speed gained to 0
	PlayerCharacter->SlideComponent->SlideSpeedGained = 0;
}

