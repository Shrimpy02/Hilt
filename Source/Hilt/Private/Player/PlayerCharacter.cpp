#include "Player/PlayerCharacter.h"

#include "Components/PlayerMovementComponent.h"
#include "Components/Camera/CameraArmComponent.h"
#include "Components/Camera/PlayerCameraComponent.h"
#include "Components/GrapplingHook/GrapplingComponent.h"
#include "Components/TerrainGun/TerrainGunComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
//#include "Components/SphereComponent.h"
#include "Components/RocketLauncherComponent.h"
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
	Camera = CreateDefaultSubobject<UPlayerCameraComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, Camera));
	CameraArm = CreateDefaultSubobject<UCameraArmComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, CameraArm));
	RocketLauncherComponent = CreateDefaultSubobject<URocketLauncherComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, RocketLauncherComponent));
	GrappleComponent = CreateDefaultSubobject<UGrapplingComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, GrappleComponent));
	RopeComponent = CreateDefaultSubobject<URopeComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, RopeComponent));
	RopeMesh = CreateDefaultSubobject<USkeletalMeshComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, RopeMesh));
	ScoreComponent = CreateDefaultSubobject<UScoreComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, ScoreComponent));

	//setup attachments
	CameraArm->SetupAttachment(GetRootComponent());
	Camera->SetupAttachment(CameraArm);
	RopeMesh->SetupAttachment(GetMesh(), FName("GrapplingHookSocket"));
	RopeComponent->SetupAttachment(RopeMesh, FName("GrapplingHookSocket"));
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
		EnhancedInputComponent->BindAction(InputDataAsset->IA_Slide, ETriggerEvent::Triggered, this, &APlayerCharacter::StartSlide);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_Slide, ETriggerEvent::Completed, this, &APlayerCharacter::StopSlide);
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

void APlayerCharacter::WasdMovement(const FInputActionValue& Value)
{
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
	if (GrappleComponent->bIsGrappling && !GrappleComponent->ShouldUseNormalMovement())
	{
		//get the up vector from the control rotation
		const FVector PlayerDirectionYaw_Upwards_Downwards = FRotationMatrix(YawPlayerRotation).GetUnitAxis(EAxis::Z);

		////get the rope direction
		//const FVector RopeDirection = RopeComponent->GetRopeDirection(0).GetSafeNormal();

		//get the X axis for the movement input
		const FVector MovementXAxis = FVector::CrossProduct(PlayerDirectionYaw_Upwards_Downwards.GetSafeNormal(), Camera->GetForwardVector()).GetSafeNormal();

		////draw a debug arrow
		//DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + MovementXAxis * 100, 100, FColor::Red, false, 0, 0, 1);

		//get the right vector from the control rotation
		const FVector PlayerDirectionYaw_Left_Right = FRotationMatrix(YawPlayerRotation).GetUnitAxis(EAxis::Y);

		//get the X axis for the movement input
		const FVector MovementYAxis = FVector::CrossProduct((PlayerDirectionYaw_Left_Right * -1).GetSafeNormal(), Camera->GetForwardVector()).GetSafeNormal();

		////draw a debug arrow
		//DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + MovementYAxis * 100, 100, FColor::Green, false, 0, 0, 1);

		//add upwards/downwards movement input
		AddMovementInput(MovementYAxis, VectorDirection.Y);

		//add left/right movement input
		AddMovementInput(MovementXAxis, VectorDirection.X);

		////print the vector direction
		//GEngine->AddOnScreenDebugMessage(99, 0.f, FColor::Red, FString::Printf(TEXT("VectorDirection: %s"), *VectorDirection.ToString()));

		return;
	}

	//get the forward vector from the control rotation
	const FVector PlayerDirectionYaw_Forward_Backward = FRotationMatrix(YawPlayerRotation).GetUnitAxis(EAxis::X);

	//get the right vector from the control rotation
	const FVector PlayerDirectionYaw_Left_Right = FRotationMatrix(YawPlayerRotation).GetUnitAxis(EAxis::Y);

	//check if we're not sliding
	if (PlayerMovementComponent->IsSliding())
	{
		//add left/right movement input
		AddMovementInput(FVector::CrossProduct(Camera->GetForwardVector(), -GetActorUpVector()), VectorDirection.X);
		//GetActorRightVector()
		//add left/right movement input

		return;
	}
	
	//add forward/backwards movement input
	AddMovementInput(PlayerDirectionYaw_Forward_Backward, VectorDirection.Y);

	//add left/right movement input
	AddMovementInput(PlayerDirectionYaw_Left_Right, VectorDirection.X);
}

void APlayerCharacter::MouseMovement(const FInputActionValue& Value)
{
	//get the look axis input
	const FVector2D LookAxisInput = Value.Get<FVector2D>();

	//add controller yaw and pitch input
	AddControllerYawInput(LookAxisInput.X);
	AddControllerPitchInput(-LookAxisInput.Y);
}

void APlayerCharacter::PauseGame(const FInputActionValue& Value)
{
	//get the player controller
	APlayerController* PC = GetLocalViewingPlayerController();

	//toggle the pause menu
	PC->SetPause(!PC->IsPaused());
}

void APlayerCharacter::FireRocketLauncher(const FInputActionValue& Value)
{
	//fire the rocket launcher
	RocketLauncherComponent->FireProjectile(Camera->GetForwardVector());
}

void APlayerCharacter::RestartGame(const FInputActionValue& Value)
{
	//check if we have a valid game mode
	if (GameMode)
	{
		//restart the game
		 GameMode->RestartLevel();

		 //call the blueprint event
		 OnPlayerRespawn();
	}
}

void APlayerCharacter::ShootGrapple(const FInputActionValue& Value)
{
	GrappleComponent->StartGrappleCheck();
}

void APlayerCharacter::StopGrapple(const FInputActionValue& Value)
{
	//stop grappling
	GrappleComponent->StopGrapple();
}

void APlayerCharacter::StartSlide(const FInputActionValue& Value)
{
	PlayerMovementComponent->StartSlide();
}

void APlayerCharacter::StopSlide(const FInputActionValue& Value)
{
	PlayerMovementComponent->StopSlide();
}

void APlayerCharacter::DoJump(const FInputActionValue& Value)
{
	//call the jump function
	Jump();
}

void APlayerCharacter::StopTheJumping(const FInputActionValue& Value)
{
	//call the stop jump function
	StopJumping();
}
