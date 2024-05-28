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

	//the rocket explosion class to spawn when the rocket hits something
	UPROPERTY(EditAnywhere, Category = "Rocket Launcher")
	TSubclassOf<AActor> RocketExplosionClass;

	//override(s)
	virtual void OnProjectileHit(AActor* Projectile, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit) override;
	
};
