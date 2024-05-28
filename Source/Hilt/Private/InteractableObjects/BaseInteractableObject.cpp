// Class Includes
#include "InteractableObjects/BaseInteractableObject.h"

// Other Includes
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

ABaseInteractableObject::ABaseInteractableObject()
{
	PrimaryActorTick.bCanEverTick = true;

	// Niagara Component -------
	// Also acts as object 
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	SetRootComponent(NiagaraComp);

	// Visible Mesh -------------
	VisibleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisibleMesh"));
	VisibleMesh->SetupAttachment(GetRootComponent());
		// Collision Settings
	VisibleMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	VisibleMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	VisibleMesh->SetGenerateOverlapEvents(false);
	VisibleMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	VisibleMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	VisibleMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	// Block Collision Mesh  -------------
	BlockerCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockerCollisionBox"));
	BlockerCollisionBox->SetupAttachment(GetRootComponent());
		// Collision Settings
	BlockerCollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	BlockerCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BlockerCollisionBox->SetGenerateOverlapEvents(false);
	BlockerCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	BlockerCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	BlockerCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

}

void ABaseInteractableObject::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseInteractableObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateVFXLocationRotation();
}

void ABaseInteractableObject::UpdateVFXLocationRotation()
{
	if (NiagaraComp)
	{
		NiagaraComp->SetWorldLocation(GetActorLocation());
		NiagaraComp->SetWorldRotation(GetActorRotation());
	}
}

void ABaseInteractableObject::PlayVFX(UNiagaraSystem* _niagaraVFX, FVector _location, FRotator _rotation)
{
	if (_niagaraVFX && NiagaraComp) {
		NiagaraComp->SetAsset(_niagaraVFX);

		NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			_niagaraVFX,
			_location,
			_rotation,
			FVector(1.f),
			true,
			true,
			ENCPoolMethod::None,
			true);
	}
}

void ABaseInteractableObject::PlayAudio(USoundBase* _soundBase, FVector _location)
{
	if (_soundBase)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			_soundBase,
			_location
		);
	}
}
