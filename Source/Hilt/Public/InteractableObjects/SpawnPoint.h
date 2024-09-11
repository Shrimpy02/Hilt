#pragma once

// Class includes
#include "CoreMinimal.h"
#include "InteractableObjects/BaseInteractableObject.h"
#include "SpawnPoint.generated.h"

// Forward Declaration`s

/**
 * @class ASpawnPoint.
 * @brief The spawn and reset point of the player in each level. 
 */
UCLASS()
class ASpawnPoint : public ABaseInteractableObject
{
	GENERATED_BODY()

public:
	//  ---------------------- Public Variable`s ----------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SpawnIndex = 0;

private:
	//  ---------------------- Private Variable`s ---------------------

	// ------------- Class components ------------

	// ------------- class Refs ------------

	// ------------- Timer Handlers ------------

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

	ASpawnPoint();

	// Function`s ----------
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;


private:
	//  --------------------- Private Function`s ----------------------



public:
	//  --------------- Getter`s / Setter`s / Adder`s -----------------

	// Getter`s -------

	// Setter`s --------

	// Adder`s --------



};
