// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Helpers/ProjectileGunComponent.h"
#include "RocketLauncherComponent.generated.h"

/**
 * 
 */
UCLASS()
class HILT_API URocketLauncherComponent : public UProjectileGunComponent
{
	GENERATED_BODY()

public:

	//constructor(s)
	URocketLauncherComponent();

	//the clip size of the rocket launcher
	UPROPERTY(EditAnywhere, Category = "Rocket Launcher")
	int32 ClipSize = 3;

	//the startign ammo for the rocket launcher
	UPROPERTY(EditAnywhere, Category = "Rocket Launcher")
	int32 StartingAmmo = 3;

	//the current ammo in the clip
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rocket Launcher")
	int32 CurrentAmmo = 3;

	//the time it takes to reload the rocket launcher clip
	UPROPERTY(EditAnywhere, Category = "Rocket Launcher")
	float ReloadTime = 2.f;

	//the time it takes to load a rocket into the rocket launcher
	UPROPERTY(EditAnywhere, Category = "Rocket Launcher")
	float LoadTime = 1.f;

	//the time the last rocket was fired
	UPROPERTY(BlueprintReadOnly, Category = "Rocket Launcher")
	float LastFireTime = 0.f;

	//the last time the rocket launcher was reloaded
	UPROPERTY(BlueprintReadOnly, Category = "Rocket Launcher")
	float LastReloadTime = 0.f;

	//whether reloading is enabled
	UPROPERTY(EditAnywhere, Category = "Rocket Launcher")
	bool bEnableReloading = false;

	//the rocket explosion class to spawn when the rocket hits something
	UPROPERTY(EditAnywhere, Category = "Rocket Launcher")
	TSubclassOf<AActor> RocketExplosionClass;
	
	//override(s)
	virtual AActor* FireProjectile(FVector Direction) override;
	virtual void OnProjectileHit(AActor* Projectile, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;

	//function(s)

	//function to load a rocket into the rocket launcher clip
	UFUNCTION(BlueprintCallable, Category = "Rocket Launcher")
	void LoadRocketClip();

	//function to reset the rocket launcher
	UFUNCTION(BlueprintCallable, Category = "Rocket Launcher")
	void ResetRocketLauncher();
};
