// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/RocketLauncherComponent.h"

URocketLauncherComponent::URocketLauncherComponent()
{
	//enable ticking
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;
	UActorComponent::SetComponentTickEnabled(true);

	//set the default values
	RocketExplosionClass = nullptr;
}

AActor* URocketLauncherComponent::FireProjectile(FVector Direction)
{
	//check if we have any ammo
	if (CurrentAmmo <= 0)
	{
		//print a message to the screen
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Out of ammo"));

		//return nullptr
		return nullptr;
	}

	//check if the last fire time is less than the reload time
	if (GetWorld()->GetTimeSeconds() - LastFireTime < LoadTime)
	{
		//print a message to the screen
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Reloading"));

		//return nullptr
		return nullptr;
	}

	//decrement the current ammo
	CurrentAmmo--;

	//set the last fire time
	LastFireTime = GetWorld()->GetTimeSeconds();

	//spawn and return the projectile
	return Super::FireProjectile(Direction);
}

void URocketLauncherComponent::OnProjectileHit(AActor* Projectile, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	//check if we should ignore the owner when checking for collisions
	if (bIgnoreOwnerCollisions && OtherActor == GetOwner())
	{
		//print a message to the screen
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Projectile hit owner"));

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

void URocketLauncherComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//call the parent implementation
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bEnableReloading)
	{
		//load a rocket into the rocket launcher
		LoadRocketClip();
	}
}

void URocketLauncherComponent::LoadRocketClip()
{
	//check if the current ammo is less than the clip size
	if (CurrentAmmo >= ClipSize)
	{
		//print a message to the screen
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, TEXT("Full Clip"));

		//set the last reload time
		LastReloadTime = GetWorld()->GetTimeSeconds();

		//prevent further execution
		return;
	}

	
	//the last reload time plus the reload time is less than the current time
	const float LocReloadTime = LastReloadTime + ReloadTime;

	//check the reload time
	if (LocReloadTime > GetWorld()->GetTimeSeconds())
	{
		//prevent further execution
		return;
	}

	//increment the current ammo
	CurrentAmmo++;

	//set the last reload time
	LastReloadTime = GetWorld()->GetTimeSeconds();
}
