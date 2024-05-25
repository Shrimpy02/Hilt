#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ProjectileActor.generated.h"

/** 
 * @class AProjectileActor.
 * @brief class to be used by a projectile component or a subclass there of to spawn a projectile.
 *
 * AProjectileActor handles the runtime logic of the projectile.
 */
UCLASS()
class AProjectileActor : public AActor
{
	GENERATED_BODY()

public:

	//the projectile movement component to use
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProjectileMovementComponent* ProjectileComponent = nullptr;

	//the mesh component to use
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UMeshComponent* MeshComponent = nullptr;

	//constructor
	AProjectileActor();
};
