#pragma once

//Includes
#include "CoreMinimal.h"
#include "InteractableObjects/BaseInteractableObject.h"
#include "KillBox.generated.h"

// Forward Declaration`s

/**
 * @class ASpawnPoint.
 * @brief The spawn and reset point of the player in each level.
 */
UCLASS()
class AKillBox : public ABaseInteractableObject
{
	GENERATED_BODY()

public:
	//  ---------------------- Public Variable`s ----------------------

private:
	//  ---------------------- Private Variable`s ---------------------

	// ------------- Class components ------------

	// ------------- class Refs ------------

	// ------------- Timer Handlers ------------

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* TriggerCollisionBox;

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

	AKillBox();

	// Function`s ----------
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

private:
	//  --------------------- Private Function`s ----------------------



public:
	//  --------------- Getter`s / Setter`s / Adder`s -----------------

	// Getter`s -------

	// Setter`s --------

	// Adder`s --------
	
};
