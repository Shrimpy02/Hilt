// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputDataAsset.generated.h"

class APlayerCharacter;
/**
 * This class is used to store the input data for the player character
 */
UCLASS()
class UInputDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inputsystem")
	UInputMappingContext* InputMappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inputsystem")
	UInputAction* IA_WasdMovement = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inputsystem")
	UInputAction* IA_MouseMovement = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inputsystem")
	UInputAction* IA_DoJump = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintreadWrite, Category = "Inputsystem")
	UInputAction* IA_StopJump = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inputsystem")
	UInputAction* IA_ShootGrapple = nullptr;

	UPROPERTY(editDefaultsOnly, BlueprintReadWrite, Category = "Inputsystem")
	UInputAction* IA_StopGrapple = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inputsystem")
	UInputAction* IA_PauseButton = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inputsystem")
	UInputAction* IA_FireGun = nullptr;

	//constructor(s)
	UInputDataAsset();
};
