#include "Components/DiveComponent.h"

#include "Components/PlayerMovementComponent.h"
#include "Player/PlayerCharacter.h"

UDiveComponent::UDiveComponent()
{
	bAutoActivate = true;
}

void UDiveComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//set the player character
	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
}

void UDiveComponent::StartDive()
{
	//set the Dive start time
	StartTime = GetWorld()->GetTimeSeconds();

	//set the player movement component's movement mode to Dive
	PlayerCharacter->PlayerMovementComponent->SetMovementMode(MOVE_Custom, ECM_Dive);

	//broadcast the start Dive event
	OnStartDive.Broadcast();
}

void UDiveComponent::StopDive()
{
	//set the player movement component's movement mode to falling
	PlayerCharacter->PlayerMovementComponent->SetMovementMode(PlayerCharacter->PlayerMovementComponent->EMovementMode::MOVE_Falling);

	//set the Dive stop time
	StopTime = GetWorld()->GetTimeSeconds();

	//broadcast the stop Dive event
	OnStopDive.Broadcast();
}

bool UDiveComponent::IsDiving() const
{
	//check if the player's player movement component is in the custom movement mode and the custom movement mode is Dive
	return PlayerCharacter->PlayerMovementComponent->MovementMode == MOVE_Custom && PlayerCharacter->PlayerMovementComponent->CustomMovementMode == ECM_Dive;
}

float UDiveComponent::GetTerminalVelMultipliers(const FVector& Vector, const FVector& Gravity, float DeltaTime)
{
	//check if we are diving and we have a valid terminal velocity curve
	if (IsDiving() && TerminalVelCurve->IsValidLowLevelFast())
	{
		//get the terminal velocity multiplier
		const float TerminalVelMultiplier = TerminalVelCurve->GetFloatValue((Vector | Gravity) / FMath::Max(Gravity.Size(), 1.f));

		return TerminalVelMultiplier;
	}

	//check if we are not diving and we have a valid after Dive terminal velocity curve
	if (!IsDiving() && AfterDiveTerminalVelCurve->IsValidLowLevelFast())
	{
		//get the after Dive terminal velocity multiplier
		const float AfterDiveTerminalVelMultiplier = AfterDiveTerminalVelCurve->GetFloatValue((Vector | Gravity) / FMath::Max(Gravity.Size(), 1.f));

		return AfterDiveTerminalVelMultiplier;
	}

	return 1;
}
