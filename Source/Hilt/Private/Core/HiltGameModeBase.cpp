
// class includes
#include "Core/HiltGameModeBase.h"
#include "InteractableObjects/BaseInteractableObject.h"
#include "InteractableObjects/LaunchPad.h"
#include "InteractableObjects/SpawnPoint.h"
#include "InteractableObjects/PylonObjective.h"
#include "NPC/Enemies/BaseEnemy.h"
#include "Hilt/Public/Core/HiltTags.h"

// Other Includes
#include "Components/RocketLauncherComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerCharacter.h"
#include "Player/ScoreComponent.h"

AHiltGameModeBase::AHiltGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHiltGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APylonObjective::StaticClass(), FoundActors);
	TotalNumObjectives = FoundActors.Num();
	TotalNumActiveObjectives = FoundActors.Num();

	RestartLevel();
}

void AHiltGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TimerShouldTick)
	{
		TotalElapsedTime += DeltaTime;
		LocalElapsedTime += DeltaTime;
		CountTime();
	}

	// Checks for num objectives and calls event logic through player
	if (UWorld* World = GetWorld())
		if (APlayerController* PC = World->GetFirstPlayerController())
			if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(PC->GetPawn()))
			{
				TArray<AActor*> FoundActors;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), APylonObjective::StaticClass(), FoundActors);

				for(AActor* actor : FoundActors)
				{
					if(actor->Tags.Contains(HiltTags::ObjectActiveTag))
					{
						NumActiveObjectives++;
					}
				}

				// One objective taken
				if(NumActiveObjectives != TotalNumActiveObjectives && NumActiveObjectives != 0)
				{
					TotalNumActiveObjectives = NumActiveObjectives;
					PlayerCharacter->OnPlayerObjectivePickedUp();
					//GEngine->AddOnScreenDebugMessage(8, 1.f, FColor::Red, FString::Printf(TEXT("One Objective taken")));
				} 

				// All objectives taken 
				if(NumActiveObjectives == 0 && doOnce)
				{
					doOnce = false;
					TotalNumActiveObjectives = TotalNumObjectives;
					PlayerCharacter->OnPlayerPickedUpAllObjectives();
					//GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Orange, FString::Printf(TEXT("All objectives taken")));
				}

				NumActiveObjectives = 0;
			}
}

void AHiltGameModeBase::RestartLevel()
{
	// Get all actors with reset functionality
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseInteractableObject::StaticClass(), FoundActors);
	//GEngine->AddOnScreenDebugMessage(-2, 5.f, FColor::Green, FString::Printf(TEXT("Num resetable things in level: %d"), FoundActors.Num()));

	// Restarts timer
	ResetTimer();

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
				if (UWorld* World = GetWorld())
					if (APlayerController* PC = World->GetFirstPlayerController())
						if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(PC->GetPawn()))
						{
							// Player location
							PlayerCharacter->SetActorLocation(spawnPoint->GetActorLocation());
							PlayerCharacter->SetActorRotation(spawnPoint->GetActorRotation());
							FRotator NewCameraRotation = spawnPoint->GetActorRotation();
							PC->SetControlRotation(NewCameraRotation);

							// Player variables
							PlayerCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;
							PlayerCharacter->RocketLauncherComponent->CurrentAmmo = PlayerCharacter->RocketLauncherComponent->StartingAmmo;
							PlayerCharacter->ScoreComponent->ResetScore();

							//array for projectile actors
							TArray<AActor*> ProjectileActors;

							//get all actors of the projectile class
							UGameplayStatics::GetAllActorsOfClass(GetWorld(), PlayerCharacter->RocketLauncherComponent->ProjectileClass, ProjectileActors);

							//reset(destroy) projectile actors
							for (AActor* Actor : ProjectileActors)
							{
								//destroy the projectile
								Actor->Destroy();
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

	doOnce = true;

}

void AHiltGameModeBase::RestartLevelBP()
{
	RestartLevel();
}

void AHiltGameModeBase::StartTimer()
{
	TimerShouldTick = true;
	TotalElapsedTime = 0.0f;
}

void AHiltGameModeBase::StopTimer()
{
	TimerShouldTick = false;
}

void AHiltGameModeBase::ResetTimer()
{
	TotalElapsedTime = 0.0f;
	LocalElapsedTime = 0.0f;
	Millisecs = 0;
	Seconds = 0;
	Minutes = 0;
}

void AHiltGameModeBase::CountTime()
{
	// Calculate the milliseconds
	Millisecs = LocalElapsedTime;

	// Calculate the seconds
	if (Millisecs >= 1.0f)
	{
		Seconds += 1;
		LocalElapsedTime = 0.0f;
	}

	// Calculate the minutes
	if(Seconds >= 60)
	{
		Minutes += 1;
		Seconds = 0;
	}

	// Debug: Print elapsed time
	//GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Orange, FString::Printf(TEXT("Minutes: %i Seconds: %i Milliseconds: %f"), Minutes, Seconds, Millisecs));
	//GEngine->AddOnScreenDebugMessage(6, 1.f, FColor::Orange, FString::Printf(TEXT("Total Elapsed time: %f"), TotalElapsedTime));
	//GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Orange, FString::Printf(TEXT("Local Elapsed time: %f"), LocalElapsedTime));
}

