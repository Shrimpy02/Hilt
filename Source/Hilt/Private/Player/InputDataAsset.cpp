#include "Player/InputDataAsset.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Player/PlayerCharacter.h"

UInputDataAsset::UInputDataAsset()
{
}

void UInputDataAsset::SetupInputData(APlayerCharacter* PlayerCharacter, UEnhancedInputComponent* EnhancedInputComponent)
{
	EnhancedInputComponent->BindAction(IA_WasdMovement, ETriggerEvent::Triggered, PlayerCharacter, &APlayerCharacter::WasdMovement);
	EnhancedInputComponent->BindAction(IA_MouseMovement, ETriggerEvent::Triggered, PlayerCharacter, &APlayerCharacter::MouseMovement);
	EnhancedInputComponent->BindAction(IA_DoJump, ETriggerEvent::Triggered, PlayerCharacter, &APlayerCharacter::Jump);
	EnhancedInputComponent->BindAction(IA_StopJump, ETriggerEvent::Triggered, PlayerCharacter, &APlayerCharacter::StopJumping);
	EnhancedInputComponent->BindAction(IA_ShootGrapple, ETriggerEvent::Triggered, PlayerCharacter, &APlayerCharacter::ShootGrapple);
	EnhancedInputComponent->BindAction(IA_StopGrapple, ETriggerEvent::Triggered, PlayerCharacter, &APlayerCharacter::StopGrapple);

	//check if we have a valid input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerCharacter->GetLocalViewingPlayerController()->GetLocalPlayer()))
	{
		//add the input mapping context
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}
}
