#include "Helpers/ProjectileActor.h"

AProjectileActor::AProjectileActor()
{
	//initialize the projectile movement component
	ProjectileComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComponent"));

	//initialize the mesh component
	MeshComponent = CreateDefaultSubobject<UMeshComponent>(TEXT("MeshComponent"));
}

