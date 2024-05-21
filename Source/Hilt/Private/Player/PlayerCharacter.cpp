#include "Player/PlayerCharacter.h"

#include "Player/PlayerMovementComponent.h"
#include "Player/Camera/CameraArmComponent.h"
#include "Player/Camera/PlayerCameraComponent.h"
#include "Player/GrapplingHook/GrapplingComponent.h"
#include "Player/TerrainGun/TerrainGunComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
//#include "Components/SphereComponent.h"
#include "Player/GrapplingHook/RopeComponent.h"

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
	//PlayerMovementComponent = CreateDefaultSubobject<UPlayerMovementComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, PlayerMovementComponent));
	Camera = CreateDefaultSubobject<UPlayerCameraComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, Camera));
	CameraArm = CreateDefaultSubobject<UCameraArmComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, CameraArm));
	TerrainGunComponent = CreateDefaultSubobject<UTerrainGunComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, TerrainGunComponent));
	RocketLauncherComponent = CreateDefaultSubobject<UProjectileGunComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, RocketLauncherComponent));
	GrappleComponent = CreateDefaultSubobject<UGrapplingComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, GrappleComponent));
	RopeComponent = CreateDefaultSubobject<URopeComponent>(GET_FUNCTION_NAME_CHECKED(APlayerCharacter, RopeComponent));

	////set the root component to be the collison shape component
	//SetRootComponent(SphereComponent);

	////enable collision (query and physics) on the collision shape component
	//SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	////set the collision profile to be pawn
	//SphereComponent->SetCollisionProfileName(FName("Pawn"));

	////set the collision response to be block all
	//SphereComponent->SetCollisionResponseToAllChannels(ECR_Block);

	////set the collision response to be overlap for the camera channel
	//SphereComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Overlap);

	////enable physics simulation on the collison shape component
	//SphereComponent->SetSimulatePhysics(true);

	////prevent the physics simulation from rotating the collision shape
	//SphereComponent->BodyInstance.bLockXRotation = true;
	//SphereComponent->BodyInstance.bLockYRotation = true;
	//SphereComponent->BodyInstance.bLockZRotation = true;

	//setup attachments
	CameraArm->SetupAttachment(GetRootComponent());
	Camera->SetupAttachment(CameraArm);

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
		EnhancedInputComponent->BindAction(InputDataAsset->IA_MouseMovement, ETriggerEvent::Triggered, this, &APlayerCharacter::MouseMovement);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_DoJump, ETriggerEvent::Triggered, this, &APlayerCharacter::DoJump);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_StopJump, ETriggerEvent::Triggered, this, &APlayerCharacter::StopTheJumping);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_ShootGrapple, ETriggerEvent::Triggered, this, &APlayerCharacter::ShootGrapple);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_StopGrapple, ETriggerEvent::Triggered, this, &APlayerCharacter::StopGrapple);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_PauseButton, ETriggerEvent::Triggered, this, &APlayerCharacter::PauseGame);
		EnhancedInputComponent->BindAction(InputDataAsset->IA_FireGun, ETriggerEvent::Triggered, this, &APlayerCharacter::FireTerrainGun);
	}

	//check if we have a valid input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalViewingPlayerController()->GetLocalPlayer()))
	{
		//add the input mapping context
		Subsystem->AddMappingContext(InputDataAsset->InputMappingContext, 0);
	}
}

void APlayerCharacter::WasdMovement(const FInputActionValue& Value)
{
	//get the vector direction from the input value
	FVector2D VectorDirection = Value.Get<FVector2D>();

	//get the control rotation and set the pitch and roll to zero
	const FRotator ControlPlayerRotationYaw = GetControlRotation();
	const FRotator YawPlayerRotation(0.f, ControlPlayerRotationYaw.Yaw, 0.f);

	//check if we're grappling
	if (GrappleComponent->bIsGrappling)
	{
		//get the up vector from the control rotation
		const FVector PlayerDirectionYaw_Upwards_Downwards = FRotationMatrix(YawPlayerRotation).GetUnitAxis(EAxis::Z);

		//get the X axis for the movement input
		const FVector MovementXAxis = FVector::CrossProduct(PlayerDirectionYaw_Upwards_Downwards.GetSafeNormal(), GrappleComponent->GetGrappleDirection()).GetSafeNormal();

		//get the right vector from the control rotation
		const FVector PlayerDirectionYaw_Left_Right = FRotationMatrix(YawPlayerRotation).GetUnitAxis(EAxis::Y);

		//get the X axis for the movement input
		const FVector MovementYAxis = FVector::CrossProduct((PlayerDirectionYaw_Left_Right * -1).GetSafeNormal(), GrappleComponent->GetGrappleDirection()).GetSafeNormal();

		//add upwards/downwards movement input
		AddMovementInput(MovementYAxis, VectorDirection.Y);

		//add left/right movement input
		AddMovementInput(MovementXAxis, VectorDirection.X);

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
	//get the look axis input
	const FVector2D LookAxisInput = Value.Get<FVector2D>();

	//add controller yaw and pitch input
	AddControllerYawInput(LookAxisInput.X);
	AddControllerPitchInput(-LookAxisInput.Y);
}

void APlayerCharacter::PauseGame()
{
	//get the player controller
	APlayerController* PC = GetLocalViewingPlayerController();

	//toggle the pause menu
	PC->SetPause(!PC->IsPaused());
}

void APlayerCharacter::FireTerrainGun()
{
	//fire the terrain gun
	TerrainGunComponent->FireProjectile(Camera->GetForwardVector());
}

//void APlayerCharacter::DoJump(const FInputActionValue& Value)
//{
//
//	//call the jump function
//	Jump();
//}
//
//void APlayerCharacter::StopJumpInput(const FInputActionValue& Value)
//{
//	//check if we can use the input
//	if (CharacterState == ECharacterState::ECS_Dead)
//	{
//		return;
//	}
//
//	//call the stop jump function
//	StopJumping();
//}

void APlayerCharacter::ShootGrapple(const FInputActionValue& Value)
{
	GrappleComponent->StartGrappleCheck();
}

void APlayerCharacter::StopGrapple(const FInputActionValue& Value)
{
	//stop grappling
	GrappleComponent->StopGrapple();
}

void APlayerCharacter::DoJump(const FInputActionValue& Value)
{
}

void APlayerCharacter::StopTheJumping(const FInputActionValue& Value)
{
}
