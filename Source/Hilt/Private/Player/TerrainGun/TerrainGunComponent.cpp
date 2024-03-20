#include "Player/TerrainGun/TerrainGunComponent.h"

UTerrainGunComponent::UTerrainGunComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTerrainGunComponent::Fire(const FVector Direction)
{
	//check if the projectile class is invalid
	if (!ProjectileClass->IsValidLowLevel())
	{
		//print an error message
		UE_LOG(LogTemp, Error, TEXT("ProjectileClass is not set in TerrainGunComponent"));

		//print a message to the screen
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ProjectileClass is not set in TerrainGunComponent"));

		//prevent further execution of this function
		return;
	}

	//get the location to spawn the projectile
	const FVector SpawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * FVector::Dist(GetComponentLocation(), GetOwner()->GetActorLocation());

	//spawn the projectile
	AActor* Projectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, GetOwner()->GetActorRotation());

	//set the projectile's velocity
	Projectile->GetRootComponent()->ComponentVelocity = Direction * ProjectileSpeed;
	
	//bind the projectile's hit event
	Projectile->OnActorHit.AddDynamic(this, &UTerrainGunComponent::OnProjectileHit);

	//timer delegate to turn the projectile into terrain
	FTimerDelegate TimerDelegate;

	//bind the function to the delegate
	TimerDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UTerrainGunComponent, OnProjectileExpired), Projectile);

	//bind the timer delegate to the timer handle
	GetWorld()->GetTimerManager().SetTimer(TerrainTimerHandle, TimerDelegate, ProjectileLifeTime, false);

	//call the OnTerrainFired delegate
	OnTerrainFired.Broadcast(Projectile, GetOwner());
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

	//call the OnTerrainCollision delegate
	OnTerrainCollision.Broadcast(Projectile, OtherActor, Hit);

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
