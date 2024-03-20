#pragma once
// Class Includes
#include "CoreMinimal.h"
#include "Characters/Enemies/BaseEnemy.h"
#include "GenFlyer.generated.h"

// Forward Declaration`s


/**
 * @class AGenFlyer.
 * @brief The generation fly`er mech`s, serves as the first enemy of the game.
 *
 * AGenFlyer Contains all specific code for the flying enemy like 3D air movement,
 * patrolling and attacking functions. 
 */
UCLASS()
class AGenFlyer : public ABaseEnemy
{
	GENERATED_BODY()
public:
	//  ---------------------- Public Variable`s ----------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool RandomPatrolPointSelection = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	double PatrolPointReachedRange = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WaitAtPatrolLocationMin = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WaitAtPatrolLocationMax = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MovementSpeed = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RotationSpeed = 0.02f;


private:
	//  ---------------------- Private Variable`s ---------------------

	UPROPERTY(VisibleDefaultsOnly, Category = "Movement")
	class AActor* CurrentPatrolTarget;

	UPROPERTY()
	TArray<AActor*> TempPatrolTargets;

	UPROPERTY(VisibleDefaultsOnly, Category = "Movement")
	bool WaitingAtPatrolPointLocation = false;

	int32 CurrentTargetIndex = -1;

	// ------------- Timer Handlers ------------

	FTimerHandle PatrolTimer;

public:
	//  ---------------------- Public Function`s ----------------------
	// Constructor`s -------
	AGenFlyer();

	// Function`s ----------
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Patrol")
	void ReachedPatrolPoint();

	UFUNCTION(BlueprintImplementableEvent, Category = "Patrol")
	void LeavingPatrolPoint();

private:
	//  --------------------- Private Function`s ----------------------
		// Movements ----------
	AActor* ChoosePatrolTarget();
	void CheckPatrolTarget();
	void PatrolTimerFinished();
	bool RotateToFace(float DeltaTime, FVector Direction);
	void MoveToTarget(float DeltaTime);


public:
	//  --------------- Getter`s / Setter`s / Adder`s -----------------

	// Getter`s -------
	

	// Setter`s --------

	
	// Adder`s --------

};
