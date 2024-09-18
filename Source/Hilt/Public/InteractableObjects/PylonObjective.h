#pragma once

#include "CoreMinimal.h"
#include "InteractableObjects/BaseInteractableObject.h"
#include "NPC/Components/GrappleableComponent.h"
#include "PylonObjective.generated.h"

/**
 * @class APylonObjective.
 * @brief An interactable object triggered on collision acting as a level objective for the player.
 */
UCLASS()
class APylonObjective : public ABaseInteractableObject
{
	GENERATED_BODY()

public:
	//  ---------------------- Public Variable`s ----------------------

	bool DisableOnce = true;

private:
	//  ---------------------- Private Variable`s ---------------------


	// ------------- Class components ------------

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* TriggerCollisionBox;

	// ------------- class Refs ------------

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

	APylonObjective();

	// Function`s ----------
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void RemoveLevelPresence() override;
	virtual void AddLevelPresence() override;

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
