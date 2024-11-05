#include "Player/PlayerCharacter.h"

#include "Components/PlayerMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/GrapplingHook/PlayerGrapplingComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
//#include "Components/SphereComponent.h"
#include "Components/DiveComponent.h"
#include "Components/RocketLauncherComponent.h"
#include "Components/SlideComponent.h"
#include "Components/GrapplingHook/PlayerGrapplingInputComponent.h"
#include "Components/GrapplingHook/HeadGrapplingComponent.h"
#include "Components/GrapplingHook/PlayerHeadGrapplingComponent.h"
#include "Components/GrapplingHook/RopeComponent.h"
#include "Core/HiltGameModeBase.h"
#include "Player/ScoreComponent.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UPlayerMovementComponent>(CharacterMovementComponentName))
{
	//Enable ticking
	PrimaryActorTick.bCanEverTick = true;

	//Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	////set rotation to follow movement
	//GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	//GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	//initialize our components
	PlayerMovementComponent = Cast<UPlayerMovementComponent>(GetCharacterMovement());
	Camera = CreateDefaultSubobject<UCameraComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, Camera));
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, CameraArm));
	RocketLauncherComponent = CreateDefaultSubobject<URocketLauncherComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, RocketLauncherComponent));
	HeadGrappleComponent = CreateDefaultSubobject<UPlayerHeadGrapplingComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, HeadGrappleComponent));
	PlayerGrappleComponent = CreateDefaultSubobject<UPlayerGrapplingComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, PlayerGrappleComponent));
	PlayerGrapplingInputComponent = CreateDefaultSubobject<UPlayerGrapplingInputComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, PlayerGrapplingInputComponent));
	RopeComponent = CreateDefaultSubobject<URopeComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, RopeComponent));
	SlideComponent = CreateDefaultSubobject<USlideComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, SlideComponent));
	DiveComponent = CreateDefaultSubobject<UDiveComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, DiveComponent));
	ScoreComponent = CreateDefaultSubobject<UScoreComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, ScoreComponent));

	//setup attachments
	CameraArm->SetupAttachment(GetRootComponent());
	Camera->SetupAttachment(CameraArm);
	RopeComponent->SetupAttachment(GetMesh(), FName("GrapplingHookSocket"));
	RocketLauncherComponent->SetupAttachment(GetRootComponent());

	////set relative location and rotation for the mesh
	//SkeletalMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -60.f));
	//SkeletalMeshComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));

	////set the mesh's collision profile to be no collision
	//SkeletalMeshComponent->SetCollisionProfileName(FName("NoCollision"));

	////set the camera arm's target offset to be above the character and a little behind
	//CameraArm->TargetOffset = FVector(0.f, 10.f, 90.f);

	//make the camera follow the controller's rotation (so it uses the rotation input from the mouse)
	CameraArm->bUsePawnControlRotation = true;

	//add the player tag
	Tags.Add(FName("Player"));
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* InInputComponent)
{
	//call the parent implementation
	Super::SetupPlayerInputComponent(InInputComponent);

	//check if we have a valid input data asset and enhanced input component
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InInputComponent); EnhancedInputComponent->IsValidLowLevel() && InputDataAsset->IsValidLowLevel())
	{
		EnhancedInputComponent->BindAction(InputDataAsset->IA_WasdMovement, ETriggerEvent::Triggered, this, &APlayerCharacter::WasdMovement);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_WasdMovement, ETriggerEvent::Completed, this, &APlayerCharacter::WasdMovement);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_MouseMovement, ETriggerEvent::Triggered, this, &APlayerCharacter::MouseMovement);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_Jump, ETriggerEvent::Triggered, this, &APlayerCharacter::DoJump);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_Jump, ETriggerEvent::Completed, this, &APlayerCharacter::StopJumping);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_Grapple, ETriggerEvent::Triggered, this, &APlayerCharacter::ShootGrapple);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_StopGrapple, ETriggerEvent::Triggered, this, &APlayerCharacter::StopGrapple);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_Slide, ETriggerEvent::Triggered, this, &APlayerCharacter::StartDiveOrSlide);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_Slide, ETriggerEvent::Completed, this, &APlayerCharacter::StopDiveOrSlide);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_FireGun, ETriggerEvent::Triggered, this, &APlayerCharacter::FireRocketLauncher);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_PauseButton, ETriggerEvent::Triggered, this, &APlayerCharacter::PauseGame);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_RestartGame, ETriggerEvent::Triggered, this, &APlayerCharacter::RestartGame);
	}

	//check if we have a valid input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalViewingPlayerController()->GetLocalPlayer()))
	{
		//add the input mapping context
		Subsystem->AddMappingContext(InputDataAsset->InputMappingContext, 0);
	}
}

void APlayerCharacter::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//get the game mode
	GameMode = GetWorld()->GetAuthGameMode<AHiltGameModeBase>();
}

void APlayerCharacter::ShowStreamingLevel(TArray<FName> LevelsToShow)
{
	//check if levels to show is empty
	if (LevelsToShow.Num() == 0)
	{
		//return to prevent further execution
		return;
	}

	//get the world's streaming levels
	TArray<ULevelStreaming*> StreamingLevels = GetWorld()->GetStreamingLevels();

	//iterate over the streaming levels
	for (ULevelStreaming* Level : StreamingLevels)
	{
		//check if the level is valid
		if (Level)
		{
			//iterate over the levels to show
			for (FName LevelName : LevelsToShow)
			{
				//check if the level is in the levels to load array
				if (Level->GetWorldAssetPackageFName().ToString().Contains(*LevelName.ToString()))
				{
					//set the next level to be visible
					Level->SetShouldBeVisible(true);

					//break the loop
					break;
				}
			}
		}
	}
}

void APlayerCharacter::HideStreamingLevel(TArray<FName> LevelsToHide)
{
	//check if levels to show is empty
	if (LevelsToHide.Num() == 0)
	{
		//return to prevent further execution
		return;
	}

	//get the world's streaming levels
	TArray<ULevelStreaming*> StreamingLevels = GetWorld()->GetStreamingLevels();

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

void APlayerCharacter::WasdMovement(const FInputActionValue& Value)
{
	hasStartedMoving = true;

	//check if we can activate input
	if (!bCanActivateInput)
	{
		//return to prevent further execution
		return;
	}

	//get the vector direction from the input value
	FVector2D VectorDirection = Value.Get<FVector2D>();

	//set the current move direction
	CurrentMoveDirection = VectorDirection;

	//check if the vector direction is zero
	if (VectorDirection.IsNearlyZero())
	{
		//return to prevent further execution
		return;
	}

	//get the control rotation and set the pitch and roll to zero
	const FRotator ControlPlayerRotationYaw = GetControlRotation();
	const FRotator YawPlayerRotation(0.f, ControlPlayerRotationYaw.Yaw, 0.f);

	//check if we're grappling
	if (HeadGrappleComponent->IsGrappling() && !PlayerGrapplingInputComponent->ShouldUseNormalMovement())
	{
		//get the up vector from the control rotation
		const FVector PlayerDirectionYaw_Upwards_Downwards = FRotationMatrix(YawPlayerRotation).GetUnitAxis(EAxis::Z);

		//get the X axis for the movement input
		const FVector MovementXAxis = FVector::CrossProduct(PlayerDirectionYaw_Upwards_Downwards.GetSafeNormal(), Camera->GetForwardVector()).GetSafeNormal();

		//get the right vector from the control rotation
		const FVector PlayerDirectionYaw_Left_Right = FRotationMatrix(YawPlayerRotation).GetUnitAxis(EAxis::Y);

		//get the Y axis for the movement input
		const FVector MovementYAxis = FVector::CrossProduct((PlayerDirectionYaw_Left_Right * -1).GetSafeNormal(), Camera->GetForwardVector()).GetSafeNormal();

		//add upwards/downwards movement input
		AddMovementInput(MovementYAxis, VectorDirection.Y);

		//add left/right movement input
		AddMovementInput(MovementXAxis, VectorDirection.X);

		return;
	}

	//check if we're sliding
	if (SlideComponent->IsSliding())
	{
		//add left/right movement input
		AddMovementInput(FVector::CrossProduct(Camera->GetForwardVector(), -GetActorUpVector()), VectorDirection.X);

		return;
	}

	//get the forward vector from the control rotation
	const FVector PlayerDirectionYaw_Forward_Backward = FRotationMatrix(YawPlayerRotation).GetUnitAxis(EAxis::X);

	//get the right vector from the control rotation
	const FVector PlayerDirectionYaw_Left_Right = FRotationMatrix(YawPlayerRotation).GetUnitAxis(EAxis::Y);
	
	//add forward/backwards movement input
	AddMovementInput(PlayerDirectionYaw_Forward_Backward, VectorDirection.Y);

	//add left/right movement input
	AddMovementInput(PlayerDirectionYaw_Left_Right, VectorDirection.X);
}

void APlayerCharacter::MouseMovement(const FInputActionValue& Value)
{
	//check if we can activate input
	if (!bCanActivateInput)
	{
		//return to prevent further execution
		return;
	}

	//get the look axis input
	const FVector2D LookAxisInput = Value.Get<FVector2D>();

	//add controller yaw and pitch input
	AddControllerYawInput(LookAxisInput.X);
	AddControllerPitchInput(-LookAxisInput.Y);
}

void APlayerCharacter::PauseGame(const FInputActionValue& Value)
{
	//check if we can activate pause
	if (!bCanActivatePause)
	{
		//return early to prevent further execution
		return;
	}

	//get the player controller
	APlayerController* PC = GetLocalViewingPlayerController();

	//toggle the pause menu
	PC->SetPause(!PC->IsPaused());
}

void APlayerCharacter::FireRocketLauncher(const FInputActionValue& Value)
{
	hasStartedMoving = true;
	//check if we can activate input
	if (!bCanActivateInput)
	{
		//return to prevent further execution
		return;
	}

	//fire the rocket launcher
	RocketLauncherComponent->FireProjectile(Camera->GetForwardVector());
}

void APlayerCharacter::RestartGame(const FInputActionValue& Value)
{
	//check if we can activate restart
	if (!bCanActivateRestart)
	{
		//return to prevent further execution
		return;
	}

	//check if we have a valid game mode
	if (GameMode)
	{
		//restart the game
		 GameMode->RestartLevel();

		 //call the blueprint event
		 OnPlayerRespawn();
		 OnPlayerRestart.Broadcast();
	}
}

void APlayerCharacter::ShootGrapple(const FInputActionValue& Value)
{
	hasStartedMoving = true;
	//check if we can grapple
	if (bCanActivateGrapple)
	{
		//return early to prevent further execution
		return;
	}

	//check if we already are grappling
	if (HeadGrappleComponent->IsGrappling())
	{
		//return early to prevent further execution
		return;
	}

	//get the grapple hit
	const FHitResult GrappleHit = HeadGrappleComponent->GetGrappleHit(HeadGrappleComponent->MaxGrappleDistance, true);

	//check if the hit result is not a valid blocking hit
	if (!GrappleHit.IsValidBlockingHit())
	{
		//return early to prevent further execution
		return;
	}

	//start grappling
	HeadGrappleComponent->StartGrapple(GrappleHit);
}

void APlayerCharacter::StopGrapple(const FInputActionValue& Value)
{
	hasStartedMoving = true;
	//check if we can grapple
	if (bCanActivateGrapple)
	{
		//return early to prevent further execution
		return;
	}

	//stop grappling
	HeadGrappleComponent->StopGrapple();
}

void APlayerCharacter::StartDiveOrSlide(const FInputActionValue& Value)
{
	hasStartedMoving = true;
	//check if we can activate input
	if (!bCanActivateInput)
	{
		//return to prevent further execution
		return;
	}

	//check if we're in the air and we're not already diving
	if (GetCharacterMovement()->IsFalling() && !DiveComponent->IsDiving())
	{
		//start the dive
		DiveComponent->StartDive();

		//return to prevent further execution
		return;
	}

	//check if we're on the ground and we're not already sliding
	if (GetCharacterMovement()->IsWalking() && !SlideComponent->IsSliding()) 
	{
		//start the slide
		SlideComponent->StartSlide();
	}
}

void APlayerCharacter::StopDiveOrSlide(const FInputActionValue& Value)
{
	hasStartedMoving = true;
	//check if we can activate input
	if (!bCanActivateInput)
	{
		//return to prevent further execution
		return;
	}

	//check if we're diving
	if (DiveComponent->IsDiving())
	{
		//stop diving
		DiveComponent->StopDive();
	}

	//check if we're sliding
	if (SlideComponent->IsSliding())
	{
		//stop sliding
		SlideComponent->StopSlide();
	}
}

void APlayerCharacter::DoJump(const FInputActionValue& Value)
{
	hasStartedMoving = true;
	//check if we can activate input
	if (!bCanActivateInput)
	{
		//return to prevent further execution
		return;
	}

	//call the jump function
	Jump();
}

void APlayerCharacter::StopTheJumping(const FInputActionValue& Value)
{
	hasStartedMoving = true;
	//check if we can activate input
	if (!bCanActivateInput)
	{
		//return to prevent further execution
		return;
	}

	//call the stop jump function
	StopJumping();
}
