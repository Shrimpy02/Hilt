#include "Components/TerrainGun/TerrainGunComponent.h"

UTerrainGunComponent::UTerrainGunComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

AActor* UTerrainGunComponent::FireProjectile(FVector Direction)
{
	//store the spawned projectile
	AActor* SpawnedProjectile =  Super::FireProjectile(Direction);

	//check if the spawned projectile is invalid
	if (!SpawnedProjectile)
	{
		//don't log anything since the parent function already logs an error
		return nullptr;
	}

	//timer delegate to turn the projectile into terrain
	FTimerDelegate TimerDelegate;

	//bind the function to the delegate
	TimerDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UTerrainGunComponent, OnProjectileExpired), SpawnedProjectile);

	//bind the timer delegate to the timer handle
	GetWorld()->GetTimerManager().SetTimer(TerrainTimerHandle, TimerDelegate, ProjectileLifeTime, false);

	//return the spawned projectile
	return SpawnedProjectile;
}

void UTerrainGunComponent::OnProjectileHit(AActor* Projectile, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	//check if the terrain class is invalid
	if (!TerrainClass->IsValidLowLevel())
	{
		//destroy the projectile
		Projectile->Destroy();

		//print an error message
		UE_LOG(LogTemp, Error, TEXT("TerrainClass is not set in TerrainGunComponent"));

		//print a message to the screen
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("TerrainClass is not set in TerrainGunComponent"));

		//prevent further execution of this function
		return;
	}

	//call the parent implementation
	Super::OnProjectileHit(Projectile, OtherActor, NormalImpulse, Hit);

	//get the location of the hit
	const FVector TerrainLocation = Hit.ImpactPoint;

	//get the rotation of the hit
	const FRotator TerrainRotation = Hit.ImpactNormal.Rotation();

	//spawn the terrain
	AActor* Terrain = GetWorld()->SpawnActor<AActor>(TerrainClass, TerrainLocation, TerrainRotation);

	//destroy the projectile
	Projectile->Destroy();

	//call the OnTerrainSpawned delegate
	OnTerrainSpawned.Broadcast(Terrain);
}

void UTerrainGunComponent::OnProjectileExpired(AActor* Projectile) const
{
	//assert that the projectile is valid
	checkfSlow(Projectile, TEXT("Projectile is not valid in TerrainGunComponent"));

	//get the location of the projectile
	const FVector TerrainLocation = Projectile->GetActorLocation();

	//get the rotation of the projectile
	const FRotator TerrainRotation = Projectile->GetActorRotation();

	//spawn the terrain
	AActor* Terrain = GetWorld()->SpawnActor<AActor>(TerrainClass, TerrainLocation, TerrainRotation);

	//call the OnTerrainSpawned delegate
	OnTerrainSpawned.Broadcast(Terrain);

	//destroy the projectile
	Projectile->Destroy();
}
