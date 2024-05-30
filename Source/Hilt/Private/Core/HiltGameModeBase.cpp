
// class includes
#include "Core/HiltGameModeBase.h"
#include "InteractableObjects/BaseInteractableObject.h"
#include "NPC/Enemies/BaseEnemy.h"
// Other Includes
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerCharacter.h"

AHiltGameModeBase::AHiltGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHiltGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

void AHiltGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHiltGameModeBase::RestartLevel()
{
	// Get all actors with reset functionality
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseInteractableObject::StaticClass(), FoundActors);
	GEngine->AddOnScreenDebugMessage(-2, 5.f, FColor::Green, FString::Printf(TEXT("Num resetable things in level: %d"), FoundActors.Num()));

	// Reset actors
	for (AActor* Object : FoundActors)
	{
		// Try all casts
		ABaseInteractableObject* InteractableObject = Cast<ABaseInteractableObject>(Object);
		ABaseEnemy* Enemy = Cast<ABaseEnemy>(Object);

			//Rest Interactable object
		if(InteractableObject && !InteractableObject->IsActive()){

			InteractableObject->AddLevelPresence();

			//Rest Enemies
		} else if (Enemy && !Enemy->IsAlive()){

			Enemy->AddLevelPresence();
		}
	}

	// Reset player
	if (UWorld* World = GetWorld())
		if (APlayerController* PC = World->GetFirstPlayerController())
			if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(PC->GetPawn()))
			{
				// rest player function goes here.
			}
				

}

void AHiltGameModeBase::RestartLevelBP()
{
	RestartLevel();
}
