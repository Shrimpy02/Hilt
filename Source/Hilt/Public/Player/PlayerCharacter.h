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

	//event for when the player restarts
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerRestart);

	//constructor with objectinitializer to override the movement component class
	explicit APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	/** Class Components  */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UPlayerCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UCameraArmComponent* CameraArm;

	UPROPERTY(BlueprintReadOnly)
	class UPlayerMovementComponent* PlayerMovementComponent;

	//the rocket launcher component that handles the shooting of the rocket projectile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class URocketLauncherComponent* RocketLauncherComponent;

	//the grappling component that handles the player's grappling hook
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UGrapplingComponent* GrappleComponent;

	//the rope component that handles the rope that connects the player to the grappling hook
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class URopeComponent* RopeComponent;

	//the secondary skeletal mesh for the rope component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USkeletalMeshComponent* RopeMesh;

	//the score component to keep track of the player's score
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UScoreComponent* ScoreComponent;

	//input data asset to use for setting up input
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UInputDataAsset* InputDataAsset;

	//the Hilt game mode
	UPROPERTY(BlueprintReadOnly)
	AHiltGameModeBase* GameMode;

	//the current movement direction (for wasd movement)
	UPROPERTY(BlueprintReadOnly)
	FVector2D CurrentMoveDirection = FVector2D::ZeroVector;

	//whether or not we can activate any input (except for pause, restart, and grapple)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanActivateInput = true;

	//wheter or not we can activate pause input
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanActivatePause = true;

	//whether or not we can activate restart input
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanActivateRestart = true;

	//whether or not we can activate the grappling hook input
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanActivateGrapple = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int PlayerSpawnPointIndex = 0;

	//blueprint event for when the level restarts
	UPROPERTY(BlueprintAssignable)
	FOnPlayerRestart OnPlayerRestart;

	//events
	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnPlayerDeath();

	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnPlayerRespawn();

	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnPlayerObjectivePickedUp();

	UFUNCTION(BlueprintImplementableEvent, Category = "Events")
	void OnPlayerPickedUpAllObjectives();

	//overrides
	virtual void SetupPlayerInputComponent(UInputComponent* InInputComponent) override;
	virtual void BeginPlay() override;

	//function to handle loading streaming levels
	UFUNCTION(BlueprintCallable)
	void ShowStreamingLevel(TArray<FName> LevelsToLoad);

	//function to handle
	UFUNCTION(BlueprintCallable)
	void HideStreamingLevel(TArray<FName> LevelsToHide);

	//input function for shooting the grappling hook
	UFUNCTION()
	void ShootGrapple(const FInputActionValue& Value);

	//input function to stop grappling
	UFUNCTION()
	void StopGrapple(const FInputActionValue& Value);

	//input function to start sliding
	UFUNCTION()
	void StartDiveOrSlide(const FInputActionValue& Value);

	//input function to stop sliding
	UFUNCTION()
	void StopDiveOrSlide(const FInputActionValue& Value);

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

	//function to fire the rocket launcher
	UFUNCTION()
	void FireRocketLauncher(const FInputActionValue& Value);

	//function to call the restart function
	UFUNCTION()
	void RestartGame(const FInputActionValue& Value);
};
