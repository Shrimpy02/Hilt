#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TerrainGunComponent.generated.h"

/** 
 * @class UTerrainGunComponent.
 * @brief component to handle the spawning of terrain projectile and the replacement of it with the actual terrain.
 *
 * UTerrainGunComponent inherits from USceneComponent, allowing it to be part of Unreal functionality and allowing the positioning of the component in the world to determine the distance from the owner root to spawn the projectile.
 */
UCLASS()
class UTerrainGunComponent : public USceneComponent
{
	GENERATED_BODY()

	public:

	//delegate types
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTerrainFired, AActor*, Projectile, AActor*, Owner);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTerrainCollision, AActor*, Projectile, AActor*, OtherActor, const FHitResult&, HitResult);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTerrainSpawned, AActor*, Terrain);

	//the projectile to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ProjectileClass = nullptr;

	//the terrain to turn the projectile into
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> TerrainClass = nullptr;

	//the distance to spawn the projectile from the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnDistance = 100.0f;

	//the speed of the projectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProjectileSpeed = 1000.0f;

	//how long the projectile will last before turning into terrain
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProjectileLifeTime = 5.0f;

	//timer handle to turn the projectile into terrain
	FTimerHandle TerrainTimerHandle;

	//delegate to handle when the projectile is fired
	UPROPERTY(BlueprintAssignable)
	FOnTerrainFired OnTerrainFired;

	//delegate to handle when the projectile collides with something
	UPROPERTY(BlueprintAssignable)
	FOnTerrainCollision OnTerrainCollision;

	//delegate to handle when the terrain is spawned
	UPROPERTY(BlueprintAssignable)
	FOnTerrainSpawned OnTerrainSpawned;

	//constructor
	UTerrainGunComponent();

	//function to fire the projectile
	UFUNCTION(BlueprintCallable)
	void Fire(FVector Direction);

	//function to handle when the projectile hits something
	UFUNCTION()
	void OnProjectileHit(AActor* Projectile, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	//function to handle when the projectile's time is up
	UFUNCTION()
	void OnProjectileExpired(AActor* Projectile) const;

};
