// Class Includes
#include "InteractableObjects/PylonObjective.h"
#include "Hilt/Public/Core/HiltTags.h"

// Other Includes
#include <Kismet/GameplayStatics.h>

#include "Player/PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "NPC/Components/GrappleableComponent.h"

// ---------------------- Constructor`s -----------------------------
APylonObjective::APylonObjective()
{
	PrimaryActorTick.bCanEverTick = true;
	Tags.Add(HiltTags::ObjectiveTag);

	// Trigger Collision Mesh  -------------
	TriggerCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerCollisionBox"));
	TriggerCollisionBox->SetupAttachment(GetRootComponent());

	// Collision Settings
	TriggerCollisionBox->SetCollisionObjectType(ECC_WorldStatic);
	TriggerCollisionBox->SetGenerateOverlapEvents(true);
	TriggerCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TriggerCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	TriggerCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	TriggerCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

// ---------------------- Public Function`s -------------------------

void APylonObjective::BeginPlay()
{
	Super::BeginPlay();

	TriggerCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &APylonObjective::OnOverlap);
}

void APylonObjective::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APylonObjective::RemoveLevelPresence()
{
	Super::RemoveLevelPresence();

	// Trigger Collision Box mesh ------------
	// Disable collision
	TriggerCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void APylonObjective::AddLevelPresence()
{
	Super::AddLevelPresence();

	// Trigger Collision Box ------------
	// Enable collision
	TriggerCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void APylonObjective::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if(Player && DisableOnce)
	{
		// if player and is being grappled stop grapple
		if (GrappleComponent->bIsGrappled)
		{
			Player->StopGrapple(0);
		}

		AHiltGameModeBase* GameMode = Cast<AHiltGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
		GameMode->NumActiveObjectives--;

		RemoveLevelPresence();
		DisableOnce = false;
	}
}

// --------------------- Private Function`s -------------------------



// ---------------- Getter`s / Setter`s / Adder`s --------------------

