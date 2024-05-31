#pragma once

// Includes
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HiltGameModeBase.generated.h"

// Forward Declaration`s

/**
 * @class AHiltGameModeBase.
 * @brief The Hilt Game-mode base, manages all game functions like level resets.
 */
UCLASS()
class AHiltGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	//  ---------------------- Public Variable`s ----------------------

private:
	//  ---------------------- Private Variable`s ---------------------

	

public:
	//  ---------------------- Public Function`s ----------------------
	// Constructor`s -------

	AHiltGameModeBase();

	// Function`s ----------
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void RestartLevel();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void RestartLevelBP();
private:
	//  --------------------- Private Function`s ----------------------


public:
	//  --------------- Getter`s / Setter`s / Adder`s -----------------

	// Getter`s -------

	// Setter`s --------

	// Adder`s --------

};
