
#pragma once

// Includes
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseInteractableObject.generated.h"

// Forward Declaration`s
class UStaticMeshComponent;
class UBoxComponent;
struct FTimerHandle;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundBase;
class UGrappleableComponent;

/**
 * @class ABaseInteractableObject.
 * @brief Is the base interactable object all other interactable objects inherit from.
 * It contains generic functionality like tag management and VFX/Audio playing code.
 */
UCLASS()
class ABaseInteractableObject : public AActor
{
	GENERATED_BODY()
public:
	//  ---------------------- Public Variable`s ----------------------

	// ------------- Class components ------------
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* VisibleMesh;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* BlockerCollisionBox;

	UPROPERTY(VisibleAnywhere)
	UGrappleableComponent* GrappleComponent;

	// ------------- VFX ------------
	UPROPERTY(EditAnywhere)
	UNiagaraComponent* NiagaraComp;

private:
	//  ---------------------- Private Variable`s ---------------------

	

public:
	//  ---------------------- Public Function`s ----------------------
	// Constructor`s -------

	ABaseInteractableObject();

	// Function`s ----------
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void RemoveLevelPresence();
	virtual void AddLevelPresence();
	virtual bool IsActive();

	// VFX ------------------------------
	// Updates all Niagara components to play at the enemies location
	UFUNCTION()
	virtual void UpdateVFXLocationRotation();

	// Plays the input Niagara VFX at location
	UFUNCTION(BlueprintCallable)
	virtual void PlayVFX(UNiagaraSystem* _niagaraVFX, FVector _location, FRotator _rotation = FRotator::ZeroRotator);

	// Audio ------------------------------
	// Plays input audio at location 
	UFUNCTION(BlueprintCallable)
	virtual void PlayAudio(USoundBase* _soundBase, FVector _location);

private:
	//  --------------------- Private Function`s ----------------------

	void ToggleActiveOrInactiveTag();

public:
	//  --------------- Getter`s / Setter`s / Adder`s -----------------

	// Getter`s -------

	// Setter`s --------

	// Adder`s --------

};