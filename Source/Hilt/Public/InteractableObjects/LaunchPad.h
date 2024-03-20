#pragma once
// Class Includes
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaunchPad.generated.h"

// Forward Declaration`s
class USkeletalMeshComponent;
class UBoxComponent;
class USphereComponent;
struct FTimerHandle;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundBase;

/**
 * @class ALaunchPad.
 * @brief An interactable object that launches actors that enter the launch zone.
 *
 * ALaunchPad takes in the collided actor and launches it in the specified direction.
 */
UCLASS()
class ALaunchPad : public AActor
{
	GENERATED_BODY()
public:
	//  ---------------------- Public Variable`s ----------------------

	UPROPERTY(EditAnywhere, Category = "Variable")
	float LaunchPadCoolDownTime = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Variable")
	FVector DefaultThrowDirection = GetActorUpVector();

	UPROPERTY(EditAnywhere, Category = "Variable")
	float DefaultThrowStrength = 400000.0f;

private:
	//  ---------------------- Private Variable`s ---------------------
	UPROPERTY(VisibleDefaultsOnly, Category = "Variable")
	bool CoolingDown = false;

	// ------------- Class components ------------
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* CollisionMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* LaunchMesh;

	// ------------- class Refs ------------


	// ------------- Timer Handlers ------------
	FTimerHandle JumpResetTimerHandeler;

	// VFX ------------------------------
	UPROPERTY(EditAnywhere)
	UNiagaraComponent* NiagaraComp;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* VFXIdle;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* VFXLaunch;

	// Audio ------------------------------
	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* IdleSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* LaunchSound;

public:
	//  ---------------------- Public Function`s ----------------------
	// Constructor`s -------

	ALaunchPad();

	// Function`s ----------
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void ThrowActor(AActor* _actor);
	void CooldownComplete();

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

	// Manages all start-overlap logic
	UFUNCTION(BlueprintCallable)
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// Manages all end-overlap logic
	UFUNCTION(BlueprintCallable)
	virtual void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Patrol")
	void ThrewAnActor();

	UFUNCTION(BlueprintImplementableEvent, Category = "Patrol")
	void ThrowCoolDownComplete();

private:
	//  --------------------- Private Function`s ----------------------



public:
	//  --------------- Getter`s / Setter`s / Adder`s -----------------

	// Getter`s -------

	// Setter`s --------

	// Adder`s --------


};
