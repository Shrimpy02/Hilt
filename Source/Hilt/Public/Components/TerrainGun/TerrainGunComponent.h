#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Helpers/ProjectileGunComponent.h"
#include "TerrainGunComponent.generated.h"

/** 
 * @class UTerrainGunComponent.
 * @brief component to handle the spawning of terrain projectile and the replacement of it with the actual terrain.
 *
 * UTerrainGunComponent inherits from UProjectileComponent, allowing it to be part of Unreal functionality and allowing the positioning of the component in the world to determine the distance from the owner root to spawn the projectile.
 */
UCLASS()
class UTerrainGunComponent : public UProjectileGunComponent
{
	GENERATED_BODY()

	public:

	//delegate types
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTerrainSpawned, AActor*, Terrain);

	//the terrain to turn the projectile into
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> TerrainClass = nullptr;

	//how long the projectile will last before turning into terrain
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProjectileLifeTime = 5.0f;

	//timer handle to turn the projectile into terrain
	FTimerHandle TerrainTimerHandle;

	//delegate to handle when the terrain is spawned
	UPROPERTY(BlueprintAssignable)
	FOnTerrainSpawned OnTerrainSpawned;

	//constructor
	UTerrainGunComponent();

	virtual AActor* FireProjectile(FVector Direction) override;
	virtual void OnProjectileHit(AActor* Projectile, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit) override;

	//function to handle when the projectile's time is up
	UFUNCTION()
	void OnProjectileExpired(AActor* Projectile) const;

};
