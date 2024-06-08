
// class includes
#include "Core/HiltGameModeBase.h"
#include "InteractableObjects/BaseInteractableObject.h"
#include "InteractableObjects/LaunchPad.h"
#include "InteractableObjects/SpawnPoint.h"
#include "NPC/Enemies/BaseEnemy.h"

// Other Includes
#include "Components/Camera/PlayerCameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerCharacter.h"

AHiltGameModeBase::AHiltGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHiltGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	RestartLevel();
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
			//Rest Interactable objects
		if(ABaseInteractableObject* InteractableObject = Cast<ABaseInteractableObject>(Object)){

			// Reset non active objects ------------
			if(!InteractableObject->IsActive())
			{
				InteractableObject->AddLevelPresence();
			}

			// Reset active objects cooldowns ------------
			else if (ALaunchPad* throwPad = Cast<ALaunchPad>(InteractableObject))
			{
				throwPad->ResetCooldown();
			}

			// Reset player to spawnpoint if there is one
			else if (ABaseInteractableObject* spawnPoint = Cast<ASpawnPoint>(Object))
			{
				if (UWorld* World = GetWorld())
					if (APlayerController* PC = World->GetFirstPlayerController())
						if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(PC->GetPawn()))
						{
							PlayerCharacter->SetActorLocation(spawnPoint->GetActorLocation());
							PlayerCharacter->SetActorRotation(spawnPoint->GetActorRotation());
							PlayerCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;
							
						}
			}
		}

		//Rest Enemies
		else if (ABaseEnemy* Enemy = Cast<ABaseEnemy>(Object)) {

			// Reset non active enemies ------------
			if(!Enemy->IsAlive())
			{
				Enemy->AddLevelPresence();
			}
		}


	}



	// Reset player
	
				

}

void AHiltGameModeBase::RestartLevelBP()
{
	RestartLevel();
}
