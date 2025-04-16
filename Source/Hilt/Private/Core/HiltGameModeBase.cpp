
// class includes
#include "Core/HiltGameModeBase.h"
#include "InteractableObjects/BaseInteractableObject.h"
#include "InteractableObjects/LaunchPad.h"
#include "InteractableObjects/SpawnPoint.h"
#include "InteractableObjects/PylonObjective.h"
#include "NPC/Enemies/BaseEnemy.h"
#include "Hilt/Public/Core/HiltTags.h"
#include "Networking.h"

// Other Includes
#include "SocketSubsystem.h"
#include "Components/RocketLauncherComponent.h"
#include "Components/GrapplingHook/PlayerHeadGrapplingComponent.h"
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

	// Adds All streaming levels to default levels array
	StreamingLevels = GetWorld()->GetStreamingLevels();
	for (ULevelStreaming* Level : StreamingLevels)
		if (Level)
			if (Level->GetLevelStreamingState() == ELevelStreamingState::LoadedVisible)
			{
				//find the part of the string after the last underscore
				FString LevelName = Level->GetWorldAssetPackageFName().ToString();
				LevelName = LevelName.RightChop(LevelName.Find("_0_") + 3);

				//add the level to the levels to show array
				DefaultLevelsToShow.Add(*LevelName);
			}
			else
			{
				//find the part of the string after the last underscore
				FString LevelName = Level->GetWorldAssetPackageFName().ToString();
				LevelName = LevelName.RightChop(LevelName.Find("_0_") + 3);

				//add the level to the levels to show array
				LevelsToHide.Add(*LevelName);
			}

	ShowAllStreamingLevels();
	
	// Gets all objectives and sets num for win condition
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APylonObjective::StaticClass(), FoundActors);
	NumActiveObjectives = FoundActors.Num();
	TotalNumActiveObjectives = FoundActors.Num();

	TArray<AActor*> SpawnActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnPoint::StaticClass(), SpawnActors);
	
	for(AActor* actor : SpawnActors)
	{
		if(ASpawnPoint* spawnPoint = Cast<ASpawnPoint>(actor))
			LevelSpawnPoints.Add(spawnPoint);
	}

	RestartLevel();
}

void AHiltGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (UWorld* World = GetWorld())
		if (APlayerController* PC = World->GetFirstPlayerController())
			if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(PC->GetPawn()))
			{
				if (PlayerCharacter->hasStartedMoving)
				{
					if (TimerShouldTick)
					{
						TotalElapsedTime += DeltaTime;
						LocalElapsedTime += DeltaTime;
						CountTime();
					}
				}
			}
	
	

	// Checks if all objectives are taken
	if (UWorld* World = GetWorld())
		if (APlayerController* PC = World->GetFirstPlayerController())
			if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(PC->GetPawn()))
			{
				// All objectives taken 
				if(NumActiveObjectives <= 0 && DoObjectivesOnce)
				{
					DoObjectivesOnce = false;
					PlayerCharacter->OnPlayerPickedUpAllObjectives();
				}
			}
}

void AHiltGameModeBase::RestartLevel()
{
	if (!canRestart) return;

	//ShowAllStreamingLevels();
	// Restarts timer
	ResetTimer();

	// RESET OBJECTIVES
	TArray<AActor*> PylonActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APylonObjective::StaticClass(), PylonActors);

	for (AActor* object : PylonActors)
	{
		if (APylonObjective* objective = Cast<APylonObjective>(object))
		{
			if (!objective->IsActive())
			{
				objective->AddLevelPresence();
				objective->DisableOnce = true;
			}
		}

		// Reset NumActiveObjectives
		NumActiveObjectives = TotalNumActiveObjectives;
	}

	// Get all actors with reset JUMPPADS
	TArray<AActor*> LaunchActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALaunchPad::StaticClass(), LaunchActors);

	for (AActor* object : LaunchActors)
	{
		if (ALaunchPad* throwPad = Cast<ALaunchPad>(object))
		{
			throwPad->ResetCooldown();
		}
	}

	// RESET SPAWNPOINT AND PLAYER
	for(ASpawnPoint* spawnPoint : LevelSpawnPoints)
	{
		if (spawnPoint)
			if (UWorld* World = GetWorld())
				if (APlayerController* PC = World->GetFirstPlayerController())
					if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(PC->GetPawn()))
						if(PlayerCharacter->PlayerSpawnPointIndex == spawnPoint->SpawnIndex)
						{
							// Player location
							PlayerCharacter->SetActorLocation(spawnPoint->GetActorLocation());
							PlayerCharacter->SetActorRotation(spawnPoint->GetActorRotation());
							FRotator NewCameraRotation = spawnPoint->GetActorRotation();
							PC->SetControlRotation(NewCameraRotation);

							// Player variables
							PlayerCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;
							PlayerCharacter->RocketLauncherComponent->ResetRocketLauncher();
							PlayerCharacter->ScoreComponent->ResetScore();
							PlayerCharacter->hasStartedMoving = false;

							//check if the player is grappling
							if (PlayerCharacter->HeadGrappleComponent->IsGrappling())
							{
								PlayerCharacter->HeadGrappleComponent->StopGrapple(false);
							}

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
						} else {
							// Player location
							PlayerCharacter->SetActorLocation(LevelSpawnPoints[0]->GetActorLocation());
							PlayerCharacter->SetActorRotation(LevelSpawnPoints[0]->GetActorRotation());
							FRotator NewCameraRotation = LevelSpawnPoints[0]->GetActorRotation();
							PC->SetControlRotation(NewCameraRotation);

							// Player variables
							PlayerCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;
							PlayerCharacter->RocketLauncherComponent->CurrentAmmo = PlayerCharacter->RocketLauncherComponent->StartingAmmo;
							PlayerCharacter->ScoreComponent->ResetScore();
							PlayerCharacter->hasStartedMoving = false;

							//check if the player is grappling
							if (PlayerCharacter->HeadGrappleComponent->IsGrappling())
							{
								PlayerCharacter->HeadGrappleComponent->StopGrapple(false);
							}

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
	

	// RESET ENEMIES
	//if (ABaseEnemy* Enemy = Cast<ABaseEnemy>(Object)) {
	//
	//	// Reset non active enemies ------------
	//	if (!Enemy->IsAlive())
	//	{
	//		Enemy->AddLevelPresence();
	//	}
	//}

	OnRestartLevelCustom();
	GetWorld()->GetTimerManager().SetTimer(RestartCooldownHandler, this, &AHiltGameModeBase::RestartCooldownComplete, RestartCooldown, false);

	DoObjectivesOnce = true;
	canRestart = false;

	HideNotDefaultStreamingLevels();
}

void AHiltGameModeBase::RestartCooldownComplete()
{
	canRestart = true;
}

void AHiltGameModeBase::ShowAllStreamingLevels()
{
	// Enables visibility on all levels
	for (ULevelStreaming* Level : StreamingLevels)
	{
		Level->SetShouldBeVisible(true);
		//if(Level->IsLevelVisible())
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("LevelVis"));
	}
}

void AHiltGameModeBase::HideNotDefaultStreamingLevels()
{
	//iterate over the streaming levels
	for (ULevelStreaming* Level : StreamingLevels)
	{
		//check if the level is valid
		if (Level)
		{
			//iterate over the levels to show
			for (FName LevelName : LevelsToHide)
			{
				//check if the level is in the levels to load array
				if (Level->GetWorldAssetPackageFName().ToString().Contains(*LevelName.ToString()))
				{
					//set the next level to be visible
					Level->SetShouldBeVisible(false);
				}
			}
		}
	}
}

bool AHiltGameModeBase::IsConnectedToInternet()
{
	bool bIsConnected = false;
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	// Create a socket for the connection attempt
	TSharedRef<FInternetAddr> InternetAddress = SocketSubsystem->CreateInternetAddr();
	bool bIsValid;
	InternetAddress->SetIp(TEXT("8.8.8.8"), bIsValid); // Google DNS IP
	InternetAddress->SetPort(53); // DNS port

	// Create a TCP socket and attempt connection
	FSocket* Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("InternetConnectionTest"), false);
	if (Socket && bIsValid)
	{
		bIsConnected = Socket->Connect(*InternetAddress); // Try to connect
		SocketSubsystem->DestroySocket(Socket); // Clean up socket after check
	}

	return bIsConnected;
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
	//LocalMillisecs = 0.0f;
	Millisecs = 0.f;
	Seconds = 0;
	Minutes = 0;
}

void AHiltGameModeBase::CountTime()
{
	// Calculate the milliseconds
	Millisecs = LocalElapsedTime;

	//Millisecs = (int)(LocalMillisecs*100);

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

