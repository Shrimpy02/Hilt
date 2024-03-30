#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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

	//event to handle when the projectile is fired
	UPROPERTY(BlueprintAssignable)
	FOnProjectileFired OnProjectileFired;

	//event to handle when the projectile hits something
	UPROPERTY(BlueprintAssignable)
	FOnProjectileCollision OnProjectileCollision;

	//constructor
	UProjectileGunComponent();

	//function to fire the projectile
	UFUNCTION(BlueprintCallable)
	virtual AActor* FireProjectile(FVector Direction);

	//function to handle when the projectile hits something
	UFUNCTION()
	virtual void OnProjectileHit(AActor* Projectile, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

};
