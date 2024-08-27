// Class Includes
#include "InteractableObjects/BaseInteractableObject.h"
#include "Core/HiltTags.h"

// Other Includes
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NPC/Components/GrappleableComponent.h"

ABaseInteractableObject::ABaseInteractableObject()
{
	PrimaryActorTick.bCanEverTick = true;

	// Niagara Component -------
	// Also acts as object 
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	SetRootComponent(NiagaraComp);

	// GrappleComp -----
	GrappleComponent = CreateDefaultSubobject<UGrappleableComponent>(TEXT("GrappleComp"));

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

	// Add Tags
	Tags.Add(HiltTags::ObjectTag);
	Tags.Add(HiltTags::ObjectActiveTag);
}

void ABaseInteractableObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateVFXLocationRotation();
}

void ABaseInteractableObject::RemoveLevelPresence()
{
	// Visible mesh ------------
	// Disable collision & visibility
	VisibleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisibleMesh->ToggleVisibility(false);

	// Blocker collision box ------------
	// Disable collision
	BlockerCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Niagara component ------------
	// Disable visibility
	NiagaraComp->ToggleVisibility(false);
	
	// Toggles between inactive and active tags'
	ToggleActiveOrInactiveTag();

}

void ABaseInteractableObject::AddLevelPresence()
{
	// Visible mesh ------------
	// Enable collision & visibility
	VisibleMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	VisibleMesh->ToggleVisibility(true);

	// Blocker collision box ------------
	// Enable collision
	BlockerCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// Niagara component ------------
	// Enable visibility
	NiagaraComp->ToggleVisibility(true);

	// Toggles between inactive and active tags
	ToggleActiveOrInactiveTag();

}

bool ABaseInteractableObject::IsActive()
{
	return Tags.Contains(HiltTags::ObjectActiveTag) ? true : false;
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

void ABaseInteractableObject::ToggleActiveOrInactiveTag()
{
	if(Tags.Contains(HiltTags::ObjectActiveTag) && !Tags.Contains(HiltTags::ObjectNotActiveTag))
	{
		Tags.Remove(HiltTags::ObjectActiveTag);
		Tags.Add(HiltTags::ObjectNotActiveTag);
	} else if (!Tags.Contains(HiltTags::ObjectActiveTag) && Tags.Contains(HiltTags::ObjectNotActiveTag))
	{
		Tags.Add(HiltTags::ObjectActiveTag);
		Tags.Remove(HiltTags::ObjectNotActiveTag);
	} else
	{
		GEngine->AddOnScreenDebugMessage(1, 10.f, FColor::Red, TEXT("Error: Duplicate active/inactive tags detected on object"));
	}
}
