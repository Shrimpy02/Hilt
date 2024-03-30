#include "Helpers/ProjectileGunComponent.h"

UProjectileGunComponent::UProjectileGunComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

AActor* UProjectileGunComponent::FireProjectile(const FVector Direction)
{
	//check if the projectile class is invalid
	if (!ProjectileClass->IsValidLowLevel())
	{
		//print an error message
		UE_LOG(LogTemp, Error, TEXT("ProjectileClass is not set in the %s component of %s"), *GetName(), *GetOwner()->GetName());

		//print a message to the screen
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ProjectileClass is not set in ProjectileGunComponent"));

		//prevent further execution of this function
		return nullptr;
	}

	//get the location to spawn the projectile
	const FVector SpawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * FVector::Dist(GetComponentLocation(), GetOwner()->GetActorLocation());

	//spawn the projectile
	AActor* Projectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, GetOwner()->GetActorRotation());
	
	//bind the projectile's hit event
	Projectile->OnActorHit.AddDynamic(this, &UProjectileGunComponent::OnProjectileHit);

	//call the OnProjectileFired delegate
	OnProjectileFired.Broadcast(Projectile, GetOwner(), Direction);

	//return the projectile
	return Projectile;
}

void UProjectileGunComponent::OnProjectileHit(AActor* Projectile, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	//call the OnProjectileCollision delegate
	OnProjectileCollision.Broadcast(Projectile, OtherActor, Hit);
}
