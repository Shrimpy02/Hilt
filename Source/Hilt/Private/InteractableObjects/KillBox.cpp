
// Includes
#include "InteractableObjects/KillBox.h"
#include "Player/PlayerCharacter.h"
#include "Components/BoxComponent.h"


AKillBox::AKillBox()
{
	PrimaryActorTick.bCanEverTick = true;

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

void AKillBox::BeginPlay()
{
	Super::BeginPlay();

	TriggerCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AKillBox::OnOverlap);
}

void AKillBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AKillBox::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (Player)
	{
		//GEngine->AddOnScreenDebugMessage(10, 1.f, FColor::Orange, FString::Printf(TEXT("Restart")));
		Player->OnPlayerDeath();
	}
}
