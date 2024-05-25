// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Helpers/ProjectileActor.h"
#include "ReplaceOnHitProjectile.generated.h"

/**
 * 
 */
UCLASS()
class HILT_API AReplaceOnHitProjectile : public AProjectileActor
{
	GENERATED_BODY()
	
public:
	AReplaceOnHitProjectile();

	//the actor to replace this projectile with
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplaceOnHit")
	TSubclassOf<AActor> ReplacementActor;

	//the collision sphere for the projectile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ReplaceOnHit")
	class USphereComponent* CollisionSphere;

	//the mesh for the projectile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ReplaceOnHit")
	class UStaticMeshComponent* Mesh;

	//the niagara system for the projectile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ReplaceOnHit")
	class UNiagaraComponent* NiagaraSystem;

	//the audio component for the projectile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ReplaceOnHit")
	class UAudioComponent* Sound;

	//the on hit event for the projectile
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

};
