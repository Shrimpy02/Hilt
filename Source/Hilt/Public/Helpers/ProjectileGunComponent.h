#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ProjectileGunComponent.generated.h"

/** 
 * @class UProjectileGunComponent.
 * @brief component to handle the spawning of Projectile projectile.
 *
 * UProjectileGunComponent inherits from USceneComponent, allowing it to be part of Unreal functionality and allowing the positioning of the component in the world to determine the distance from the owner root to spawn the projectile.
 */
UCLASS()
class UProjectileGunComponent : public USceneComponent
{
	GENERATED_BODY()

	public:

	//delegate types
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnProjectileFired, AActor*, Projectile, AActor*, Owner, FVector, Direction);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnProjectileCollision, AActor*, Projectile, AActor*, OtherActor, const FHitResult&, HitResult);

	//the projectile to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ProjectileClass = nullptr;

	//whether or not to add the owner's velocity to the projectile's velocity when firing the projectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAddOwnerVelocity = true;

	//whether or not to ignore the owner when checking for collisions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIgnoreOwnerCollisions = true;

	//whether or not to allow alternative actions when the player is firing the projectile while grappled
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowAlternativeActions = true;

	//storage for the owner of this component as a player character
	UPROPERTY(BlueprintReadOnly)
	class APlayerCharacter* PlayerCharacter = nullptr;

	//event to handle when the projectile is fired
	UPROPERTY(BlueprintAssignable)
	FOnProjectileFired OnProjectileFired;

	//event to handle when the projectile hits something
	UPROPERTY(BlueprintAssignable)
	FOnProjectileCollision OnProjectileCollision;

	//constructor
	UProjectileGunComponent();

	//override(s)
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void SetInitialProjectileSpeed(FVector Direction, UProjectileMovementComponent* ProjectileMovementComponent);

	//function to fire the projectile
	UFUNCTION(BlueprintCallable)
	virtual AActor* FireProjectile(FVector Direction);

	//function to handle when the projectile hits something
	UFUNCTION()
	virtual void OnProjectileHit(AActor* Projectile, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

};
