// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/RocketLauncherComponent.h"

URocketLauncherComponent::URocketLauncherComponent()
{
	//set the default values
	RocketExplosionClass = nullptr;
}

void URocketLauncherComponent::OnProjectileHit(AActor* Projectile, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	//check if we should ignore the owner when checking for collisions
	if (bIgnoreOwnerCollisions && OtherActor == GetOwner())
	{
		//print a message to the screen
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Projectile hit owner"));

		//prevent further execution of this function
		return;
	}

	//destroy the projectile
	Projectile->Destroy();

	//check if the RocketExplosionClass is valid
	if (RocketExplosionClass->IsValidLowLevelFast())
	{
		//spawn the rocket explosion
		GetWorld()->SpawnActor<AActor>(RocketExplosionClass, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}

	//call the parent implementation
	Super::OnProjectileHit(Projectile, OtherActor, NormalImpulse, Hit);
}
