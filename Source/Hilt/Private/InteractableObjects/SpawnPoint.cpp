
// Class Includes
#include "InteractableObjects/SpawnPoint.h"

// Other Includes
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

// ---------------------- Constructor`s -----------------------------

ASpawnPoint::ASpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;

	// Visible Mesh --------------
		// Collision Settings	
	VisibleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisibleMesh->SetGenerateOverlapEvents(false);
	VisibleMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	VisibleMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// Block Collision Mesh  -------------
		// Collision Settings
	BlockerCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BlockerCollisionBox->SetGenerateOverlapEvents(false);
	BlockerCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
}

void ASpawnPoint::BeginPlay()
{
	Super::BeginPlay();
}

void ASpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// ---------------------- Public Function`s -------------------------



// --------------------- Private Function`s -------------------------


// ---------------- Getter`s / Setter`s / Adder`s --------------------
