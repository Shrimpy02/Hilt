#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
//#include "Components/PlayerMovementComponent.h"
#include "InputDataAsset.h"
#include "Core/HiltGameModeBase.h"
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

	//the terrain gun component that handles the shoting of the terrain projectile, and turning that projectile into terrain
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UTerrainGunComponent* TerrainGunComponent;

	//the rocket launcher component that handles the shooting of the rocket projectile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class URocketLauncherComponent* RocketLauncherComponent;

	//the grappling component that handles the player's grappling hook
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UGrapplingComponent* GrappleComponent;

	//the rope component that handles the rope that connects the player to the grappling hook
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class URopeComponent* RopeComponent;

	//input data asset to use for setting up input
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UInputDataAsset* InputDataAsset;

	//the Hilt game mode
	UPROPERTY(BlueprintReadOnly)
	AHiltGameModeBase* GameMode;

	//the current movement direction (for wasd movement)
	UPROPERTY(BlueprintReadOnly)
	FVector2D CurrentMoveDirection = FVector2D::ZeroVector;

	//overrides
	virtual void SetupPlayerInputComponent(UInputComponent* InInputComponent) override;
	virtual void BeginPlay() override;

	//input function for shooting the grappling hook
	UFUNCTION()
	void ShootGrapple(const FInputActionValue& Value);

	//input function to stop grappling
	UFUNCTION()
	void StopGrapple(const FInputActionValue& Value);

	//input function to jump
	UFUNCTION()
	void DoJump(const FInputActionValue& Value);

	//input function to stop jumping
	UFUNCTION()
	void StopTheJumping(const FInputActionValue& Value);

	//input function to move around using wasd
	UFUNCTION()
	void WasdMovement(const FInputActionValue& Value);

	//input function to look around using the mouse
	UFUNCTION()
	void MouseMovement(const FInputActionValue& Value);

	//function to toggle pausing of the game
	UFUNCTION()
	void PauseGame(const FInputActionValue& Value);

	//function to fire the terrain gun
	UFUNCTION()
	void FireTerrainGun(const FInputActionValue& Value);

	//function to fire the rocket launcher
	UFUNCTION()
	void FireRocketLauncher(const FInputActionValue& Value);

	//function to call the restart function
	UFUNCTION()
	void RestartGame(const FInputActionValue& Value);
};
