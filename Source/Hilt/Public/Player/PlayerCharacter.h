#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
//#include "Components/PlayerMovementComponent.h"
#include "InputDataAsset.h"
#include "PlayerCharacter.generated.h"

class AObjectivePoint;
struct FInputActionValue;

UCLASS()
class APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	//constructor with objectinitializer to override the movement component class
	explicit APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	/** Class Components  */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UPlayerCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UCameraArmComponent* CameraArm;

	UPROPERTY(BlueprintReadOnly)
	class UPlayerMovementComponent* PlayerMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UTerrainGunComponent* TerrainGunComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UGrapplingComponent* GrapplingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class URopeComponent* RopeComponent;

	//input data asset
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UInputDataAsset* InputDataAsset;
	

	//overrides
	virtual void SetupPlayerInputComponent(UInputComponent* InInputComponent) override;

	UFUNCTION()
	void ShootGrapple(const FInputActionValue& Value);

	UFUNCTION()
	void StopGrapple(const FInputActionValue& Value);

	/**
	 * Movement Functions
	*/
	UFUNCTION()
	void WasdMovement(const FInputActionValue& Value);

	UFUNCTION()
	void MouseMovement(const FInputActionValue& Value);

	/**
	 * Jumping Functions
	 */
	//UFUNCTION()
	//void DoJump(const FInputActionValue& Value);

	//UFUNCTION()
	//void StopJumpInput(const FInputActionValue& Value);
};
