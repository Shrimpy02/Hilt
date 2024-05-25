// Fill out your copyright notice in the Description page of Project Settings.


#include "ReplaceOnHitProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"

AReplaceOnHitProjectile::AReplaceOnHitProjectile()
{
	//create components
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	NiagaraSystem = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraSystem"));
	Sound = CreateDefaultSubobject<UAudioComponent>(TEXT("Sound"));

	//setup attachments
	SetRootComponent(CollisionSphere);
	Mesh->SetupAttachment(CollisionSphere);
	NiagaraSystem->SetupAttachment(CollisionSphere);
	Sound->SetupAttachment(CollisionSphere);

	//set up collision
	CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
	CollisionSphere->OnComponentHit.AddDynamic(this, &AReplaceOnHitProjectile::OnHit);
}

void AReplaceOnHitProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	//spawn the replacement actor
	if (ReplacementActor)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		AActor* NewActor = GetWorld()->SpawnActor<AActor>(ReplacementActor, GetActorTransform(), SpawnParams);

		//destroy this actor
 		Destroy();
	}
}
