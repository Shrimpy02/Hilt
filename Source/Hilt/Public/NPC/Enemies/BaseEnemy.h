#pragma once
// Class Includes
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseEnemy.generated.h"

// Forward Declaration`s
class UNiagaraComponent;
class UNiagaraSystem;
class USoundBase;
class AActor;
class USkeletalMeshComponent;
class UCapsuleComponent;

// Class Enums ---
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	EES_Idle UMETA(DisplayName = "Idle"),
	EES_Occupied UMETA(DisplayName = "Occupied"),
	EES_Patroling UMETA(DisplayName = "Patroling"),
	EES_Attacking UMETA(DisplayName = "Attacking")
};

/**
 * @class ABaseEnemy.
 * @brief Represents a base Enemy Class enemies can derive from. 
 *
 * ABaseEnemy Contains all basic function and variables enemies may need.
 * Things like health, death, getters and setters.
 */
UCLASS()
class ABaseEnemy : public AActor
{
    GENERATED_BODY()
public:
    //  ---------------------- Public Variable`s ----------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool CanDie = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool CanMove = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	int Health = 1;

	EEnemyState EnemyState = EEnemyState::EES_Idle;

private:
    //  ---------------------- Private Variable`s ---------------------

	// ------------- Class components ------------
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* VisibleMesh;

	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* CollisionMesh;

	// ------------- class Refs ------------
	UPROPERTY(VisibleAnywhere)
	 AActor* CombatTarget;

	// ------------- Timer Handlers ------------



	// VFX ------------------------------
	UPROPERTY(EditAnywhere)
	UNiagaraComponent* NiagaraComp;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* VFXChargeUp;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* VFXCoolDown;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* VFXAttack;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* VFXDie;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* VFXSpawn;

	// Audio ------------------------------
	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* GetHitSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* AttackSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* ChargeUpSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* CoolDownSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* DieSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* SpawnSound;

public:
    //  ---------------------- Public Function`s ----------------------
    // Constructor`s -------

    ABaseEnemy();

    // Function`s ----------
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

	virtual void RemoveLevelPresence();
	virtual void AddLevelPresence();
	virtual bool IsAlive();

	// Retrieves the result of take damage from UKismet system
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;


	// Checks if a point / target is within range of self and returns true if is and false if it is not
	UFUNCTION(BlueprintCallable)
	bool IsTargetPosWithinRange(const FVector& _targetLocation, const float _radiusFromSelfToCheck);
	
	// Manages all start-overlap logic
    UFUNCTION(BlueprintCallable)
    virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

	// Manages all end-overlap logic
    UFUNCTION(BlueprintCallable)
    virtual void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

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
	// Returns an FVector point, Distance away from a location in the direction of the rotation input
	UFUNCTION(BlueprintCallable)
	virtual FVector GetPointWithRotator(const FVector& Start, const FRotator& Rotation, float Distance);

	// Returns the forward FVector from an input rotation
	UFUNCTION(BlueprintCallable)
	virtual FVector GetForwardVectorOfRotation(const FRotator& Rotation);

	// Returns the FVector pointing from point 1, to point 2
	UFUNCTION(BlueprintCallable)
	virtual	FVector GetVectorBetweenTwoPoints(const FVector& Point1, const FVector& Point2);

	// Returns the distance between two input points as a float
	UFUNCTION(BlueprintCallable)
	virtual	float GetDistanceBetweenTwoPoints(const FVector& Point1, const FVector& Point2);

	// Gets the AI state from the state controller
	FORCEINLINE EEnemyState GetAIState() const { return EnemyState; }


    // Setter`s --------

	FORCEINLINE void SetCombatTarget(AActor* _combatTarget) { CombatTarget = _combatTarget; }

    // Adder`s --------

};
