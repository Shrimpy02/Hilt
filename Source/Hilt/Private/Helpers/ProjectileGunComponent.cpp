#include "Helpers/ProjectileGunComponent.h"

#include "Components/GrapplingHook/GrapplingComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NPC/Components/GrappleableComponent.h"
#include "Player/PlayerCharacter.h"

UProjectileGunComponent::UProjectileGunComponent()
{
}

void UProjectileGunComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//cast the owner to a player character
	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
}

void UProjectileGunComponent::SetInitialProjectileSpeed(const FVector Direction, UProjectileMovementComponent* ProjectileMovementComponent)
{
	//check if we should add the owner's velocity to the projectile's velocity
	if (bAddOwnerVelocity)
	{
		//set the projectile's velocity
		ProjectileMovementComponent->Velocity = Direction * (ProjectileMovementComponent->InitialSpeed + GetOwner()->GetVelocity().Size());
	}
	else
	{
		//set the projectile's velocity
		ProjectileMovementComponent->Velocity = Direction * ProjectileMovementComponent->InitialSpeed;
	}
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

	//check if the projectile has a projectile movement component
	if (UProjectileMovementComponent* ProjectileMovementComponent = Projectile->FindComponentByClass<UProjectileMovementComponent>())
	{
		//check if we should add the owner's velocity to the projectile's velocity
		if (bAddOwnerVelocity)
		{
			//set the initial projectile speed
			SetInitialProjectileSpeed(Direction, ProjectileMovementComponent);
		}
		else
		{
			//set the initial projectile speed
			SetInitialProjectileSpeed(Direction, ProjectileMovementComponent);
		}
	}

	//call the OnProjectileFired delegate
	OnProjectileFired.Broadcast(Projectile, GetOwner(), Direction);

	//return the projectile
	return Projectile;
}

void UProjectileGunComponent::OnProjectileHit(AActor* Projectile, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	//check if we should ignore the owner when checking for collisions
	if (bIgnoreOwnerCollisions && OtherActor == GetOwner())
	{
		//prevent further execution of this function
		return;
	}

	//call the OnProjectileCollision delegate
	OnProjectileCollision.Broadcast(Projectile, OtherActor, Hit);
}
