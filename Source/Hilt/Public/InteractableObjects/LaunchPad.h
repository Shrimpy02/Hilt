#pragma once

// Class includes
#include "CoreMinimal.h"
#include "InteractableObjects/BaseInteractableObject.h"
#include "LaunchPad.generated.h"

// Forward Declaration`s

/**
 * @class ALaunchPad.
 * @brief An interactable object that launches actors that enter the launch zone.
 *
 * ALaunchPad takes in the collided actor and launches it in the specified direction.
 */
UCLASS()
class ALaunchPad : public ABaseInteractableObject
{
	GENERATED_BODY()

public:
	//  ---------------------- Public Variable`s ----------------------

	UPROPERTY(EditAnywhere, Category = "Variables")
	float LaunchPadCoolDownTime = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Variables")
	FVector RelativeThrowDirection = FVector(0,0,1);

	UPROPERTY(EditAnywhere, Category = "Variables")
	float DefaultThrowStrength = 400000.0f;

private:
	//  ---------------------- Private Variable`s ---------------------
	UPROPERTY(VisibleDefaultsOnly, Category = "Variables")
	bool CoolingDown = false;

	// ------------- Class components ------------

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* TriggerCollisionBox;

	// ------------- class Refs ------------

	// ------------- Timer Handlers ------------
	FTimerHandle MainTimerHandler;

	// VFX ------------------------------
	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* VFXIdle;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* VFXAction;

	// Audio ------------------------------
	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* IdleSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* ActionSound;

public:
	//  ---------------------- Public Function`s ----------------------
	// Constructor`s -------

	ALaunchPad();

	// Function`s ----------
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void RemoveLevelPresence() override;
	virtual void AddLevelPresence() override;
	void ResetCooldown();

	UFUNCTION(BlueprintCallable)
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent, Category = "Patrol")
	void ThrewAnActor();

	UFUNCTION(BlueprintImplementableEvent, Category = "Patrol")
	void ThrowCoolDownComplete();

private:
	//  --------------------- Private Function`s ----------------------

	FVector CalcThrowDirection();
	void ThrowActor(AActor* _actor);
	void CooldownComplete();

public:
	//  --------------- Getter`s / Setter`s / Adder`s -----------------

	// Getter`s -------

	// Setter`s --------

	// Adder`s --------



};
