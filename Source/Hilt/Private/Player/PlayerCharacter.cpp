#include "Player/PlayerCharacter.h"

//Components
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/PlayerMovementComponent.h"
#include "Player/Camera/CameraArmComponent.h"
#include "Player/Camera/PlayerCameraComponent.h"
#include "Player/GrapplingHook/GrapplingComponent.h"
#include "Player/TerrainGun/TerrainGunComponent.h"
#include "EnhancedInputComponent.h"


APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer.SetDefaultSubobjectClass<UPlayerMovementComponent>(CharacterMovementComponentName))
{
	//Enable ticking
	PrimaryActorTick.bCanEverTick = true;

	//Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	//set rotation to follow movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	//create our components
	CameraArm = CreateDefaultSubobject<UCameraArmComponent>(TEXT("CameraArm"));
	Camera = CreateDefaultSubobject<UPlayerCameraComponent>(TEXT("Camera"));
	TerrainGunComponent = CreateDefaultSubobject<UTerrainGunComponent>(TEXT("TerrainGunComponent"));
	GrapplingComponent = CreateDefaultSubobject<UGrapplingComponent>(TEXT("GrapplingComponent"));

	//setup attachments
	CameraArm->SetupAttachment(GetRootComponent());
	Camera->SetupAttachment(CameraArm);

	//set relative location and rotation for the mesh
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -60.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));

	//set the mesh's collision profile to be no collision
	GetMesh()->SetCollisionProfileName(FName("NoCollision"));

	//set the capsule component's default size
	GetCapsuleComponent()->SetCapsuleHalfHeight(60);
	GetCapsuleComponent()->SetCapsuleRadius(10);

	//set the camera arm's target offset to be above the character and a little behind
	CameraArm->TargetOffset = FVector(0.f, 10.f, 90.f);

	//make the camera follow the controller's rotation (so it uses the rotation input from the mouse)
	CameraArm->bUsePawnControlRotation = true;

	//disable busepawncontrolrotation on the camera
	Camera->bUsePawnControlRotation = false;

	//default to automatically possessing the player
	AutoPossessPlayer = EAutoReceiveInput::Player0;

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
		InputDataAsset->SetupInputData(this, EnhancedInputComponent);
	}
}

void APlayerCharacter::WasdMovement(const FInputActionValue& Value)
{
	//check if the value is zero
	if (!Value.IsNonZero())
	{
		//if it is, return
		return;
	}

	//get the vector direction from the input value
	const FVector2D VectorDirection = Value.Get<FVector2D>();

	//get the control rotation and set the pitch and roll to zero
	const FRotator ControlPlayerRotationYaw = GetControlRotation();
	const FRotator YawPlayerRotation(0.f, ControlPlayerRotationYaw.Yaw, 0.f);

	//check if we're grappling
	if (GrapplingComponent->bIsGrappling)
	{
		//get the up vector from the control rotation
		const FVector PlayerDirectionYaw_Upwards_Downwards = FRotationMatrix(YawPlayerRotation).GetUnitAxis(EAxis::Z);

		//get the X axis for the movement input
		const FVector MovementXAxis = FVector::CrossProduct(PlayerDirectionYaw_Upwards_Downwards.GetSafeNormal(), GrapplingComponent->GetGrappleDirection()).GetSafeNormal();

		//get the right vector from the control rotation
		const FVector PlayerDirectionYaw_Left_Right = FRotationMatrix(YawPlayerRotation).GetUnitAxis(EAxis::Y);

		//get the X axis for the movement input
		const FVector MovementYAxis = FVector::CrossProduct((PlayerDirectionYaw_Left_Right * -1).GetSafeNormal(), GrapplingComponent->GetGrappleDirection()).GetSafeNormal();

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
	//check if we can use the input
	if (Value.IsNonZero())
	{
		//get the look axis input
		const FVector2D LookAxisInput = Value.Get<FVector2D>();

		//add controller yaw and pitch input
		AddControllerYawInput(LookAxisInput.X);
		AddControllerPitchInput(-LookAxisInput.Y);
	}
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
	GrapplingComponent->StartGrappleCheck();
}

void APlayerCharacter::StopGrapple(const FInputActionValue& Value)
{
	//stop grappling
	GrapplingComponent->StopGrapple();
}

//void APlayerCharacter::SpawnGrappleProjectile()
//{
//	//check if we have a valid grappling hook head class
//	if (GrappleHookHeadClass)
//	{
//		//get the forward vector of the camera
//		const FVector CamForwardVec = Camera->GetForwardVector();
//
//		//get the spawn rotation
//		const FRotator SpawnRotation = CamForwardVec.Rotation();
//
//		//get the spawn location
//		const FVector SpawnLocation = GetActorLocation() + CamForwardVec * GrappleSpawnDist;
//
//		//spawn parameters
//		FActorSpawnParameters SpawnParams;
//		SpawnParams.Owner = this;
//		SpawnParams.Instigator = this;
//		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
//
//		//assign the grappling hook head reference to the spawned grappling hook head
//		GrapplingHookRef = GetWorld()->SpawnActor<AGrapplingHookHead>(GrappleHookHeadClass, SpawnLocation,
//			SpawnRotation, SpawnParams);
//
//		//stop the player grapple
//		PlayerMovementComponent->StopGrapple();
//
//		//call the blueprint event
//		OnShootGrapple();
//	}
//}
