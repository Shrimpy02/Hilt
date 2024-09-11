#pragma once

// Includes
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HiltGameModeBase.generated.h"

// Forward Declaration`s
class ASpawnPoint;

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

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Variables-Time")
	float TotalElapsedTime = 0.0f;
	float LocalElapsedTime = 0.0f;

	UPROPERTY()
	float LocalMillisecs = 0;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Variables-Time")
	float Millisecs = 0;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Variables-Time")
	int Seconds = 0;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Variables-Time")
	int Minutes = 0;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Variables-Time")
	bool TimerShouldTick = true;
	FTimerHandle TimerHandler;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Variables-Time")
	float RestartCooldown = 0.3f;
	bool canRestart = true;
	FTimerHandle RestartCooldownHandler;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Variables-Game")
	int TotalNumActiveObjectives = 0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Variables-Game")
	int NumActiveObjectives = 0;

	TArray<ULevelStreaming*> StreamingLevels;
	TArray<FName> DefaultLevelsToShow;
	TArray<FName> LevelsToHide;
	TArray<ASpawnPoint*> LevelSpawnPoints;
	bool DoObjectivesOnce = true;

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
	void RestartCooldownComplete();

	void ShowAllStreamingLevels();
	void HideNotDefaultStreamingLevels();

	// Timer -----

	void CountTime();
	UFUNCTION(BlueprintCallable)
	void StartTimer();
	UFUNCTION(BlueprintCallable)
	void ResetTimer();
	UFUNCTION(BlueprintCallable)
	void StopTimer();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void RestartLevelBP();

	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void OnRestartLevelCustom();

private:
	//  --------------------- Private Function`s ----------------------


public:
	//  --------------- Getter`s / Setter`s / Adder`s -----------------

	// Getter`s -------

	// Setter`s --------

	// Adder`s --------

};
