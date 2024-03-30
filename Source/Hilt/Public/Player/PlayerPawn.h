#pragma once

#include "CoreMinimal.h"
#include "InputDataAsset.h"
#include "PlayerPawn.generated.h"

class AObjectivePoint;
struct FInputActionValue;

UCLASS()
class APlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	/** Class Components  */

	//the player movement component that handles movement logic
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UPlayerMovementComponent* PlayerMovementComponent;

	//the mesh component for the player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USkeletalMeshComponent* SkeletalMeshComponent;

	//the shape component for the player's collision shape
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USphereComponent* SphereComponent;

	//the camera component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UPlayerCameraComponent* Camera;

	//the camera arm component that holds the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCameraArmComponent* CameraArm;

	//the terrain gun component that handles the shoting of the terrain projectile, and turning that projectile into terrain
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UTerrainGunComponent* TerrainGunComponent;

	//the rocket launcher component that handles the shooting of the rocket projectile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UProjectileGunComponent* RocketLauncherComponent;

	//the grappling component that handles the player's grappling hook
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UGrapplingComponent* GrappleComponent;

	//the rope component that handles the rope that connects the player to the grappling hook
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class URopeComponent* RopeComponent;

	//input data asset to use for setting up input
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UInputDataAsset* InputDataAsset;

	//constructor with objectinitializer to override the movement component class
	APlayerPawn();

	//overrides
	virtual void SetupPlayerInputComponent(UInputComponent* InInputComponent) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;


	//input function for shooting the grappling hook
	UFUNCTION()
	void ShootGrapple(const FInputActionValue& Value);

	//input function to stop grappling
	UFUNCTION()
	void StopGrapple(const FInputActionValue& Value);

	//input function to jump
	UFUNCTION()
	void Jump(const FInputActionValue& Value);

	//input function to stop jumping
	UFUNCTION()
	void StopJumping(const FInputActionValue& Value);

	//input function to move around using wasd
	UFUNCTION()
	void WasdMovement(const FInputActionValue& Value);

	//input function to look around using the mouse
	UFUNCTION()
	void MouseMovement(const FInputActionValue& Value);

	//function to toggle pausing of the game
	UFUNCTION()
	void PauseGame();

	//function to fire the terrain gun
	UFUNCTION()
	void FireTerrainGun();
};
