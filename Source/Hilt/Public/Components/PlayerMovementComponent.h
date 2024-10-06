// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CollisionQueryParams.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PlayerMovementComponent.generated.h"

class APlayerCharacter;
class UPlayerCameraComponent;
class AGrapplingHookHead;

//custom movement modes
UENUM()
enum ECustomMovementMode
{
	ECM_None = 0,
	ECM_Grapple = 1,
	ECM_Slide = 2,
	ECM_Dive = 3
};

/**
 * Movement component for the player character that extends the default character movement component
 */
UCLASS(Blueprintable)
class UPlayerMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	//event declaration(s)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerImpulse, FVector, Impulse, bool, bVelocityChange);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerStartDive);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerStopDive);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerSuperJump);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerNormalJump);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerStartFall, const FVector&, PreviousFloorImpactNormal, const FVector&, PreviousFloorContactNormal, const FVector&, PreviousLocation);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerHitObjective);

	//reference to the player as a PlayerCharacter
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	APlayerCharacter* PlayerCharacter = nullptr;

	//the max movement speed when falling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling")
	float MaxFallSpeed = 6000;

	//the minimum speed to launch the character off of a collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	float MinCollisionLaunchSpeed = 2000;

	//the maximum speed to launch the character off of a collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	float MaxCollisionLaunchSpeed = 4000;

	//the dot product to use for what is considered a head on collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	float HeadOnCollisionDot = -0.3f;
	
	//the float value to subtract from normals when calculating whether or not a collision should be a collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	float CollisionNormalSubtract = 0.1f;

	//the speed threshold to use for when to apply a collision launch (applied in 2D using the XY plane)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	float CollisionSpeedThreshold = 1000;

	//the extra upwards force to apply when launching off of a collision when sliding
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	float SlideCollisionLaunchExtraForce = 500;

	//the float curve to use when applying the collision launch speed based on the speed of the player (0 = min speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* CollisionLaunchSpeedCurve = nullptr;

	//the float curve to use for braking deceleration when sliding based on the speed of the player (0 = min speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* WalkingBrakingFrictionCurve = nullptr;

	//the float curve to use for applying braking deceleration when falling (0 = min speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* FallingBrakingDecelerationCurve = nullptr;

	//the player's current speed limit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedLimit = 4000;

	//the max acceleration to apply when walking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	float MaxWalkingAcceleration = 1000;

	//the distance to trace for to check if the player is bunny hopping (in which case we don't want them to have falling speed or other physics applied from falling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Jumping / Falling")
	float AvoidBunnyJumpTraceDistance = 1000;

	//whether the player has gone far enough above the ground to be considered not bunny hopping
	UPROPERTY(BlueprintReadOnly, Category = "Character Movement: Jumping / Falling")
	bool bMightBeBunnyJumping = true;

	//blueprint event(s)
	UPROPERTY(BlueprintAssignable, Category = "Movement")
	FOnPlayerImpulse OnPlayerImpulse;

	UPROPERTY(BlueprintAssignable, Category = "Movement")
	FOnPlayerNormalJump OnPlayerNormalJump;

	UPROPERTY(BlueprintAssignable, Category = "Movement")
	FOnPlayerStartFall OnPlayerStartFall;

	UPROPERTY(BlueprintAssignable, Category = "Movement")
	FOnPlayerHitObjective OnPlayerHitObjective;

	//constructor
	UPlayerMovementComponent();

	//function to apply the speed limit to a velocity (if speed limit is enabled)
	UFUNCTION(BlueprintCallable, Category = "Movement")
	FVector ApplySpeedLimit(const FVector& InVelocity) const;

	//function to get the current speed limit (taking into account the current score multiplier)
	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetCurrentSpeedLimit() const;

	//override functions
	virtual void BeginPlay() override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysFalling(float deltaTime, int32 Iterations) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	//virtual bool IsWalkable(const FHitResult& Hit) const override;
	virtual void PerformMovement(float DeltaTime) override;
	virtual void HandleWalkingOffLedge(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta) override;
	virtual FVector NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const override;
	virtual float GetGravityZ() const override;
	virtual FVector GetAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration) override;
	virtual void StartFalling(int32 Iterations, float remainingTime, float timeTick, const FVector& Delta, const FVector& subLoc) override;
	virtual FVector ConsumeInputVector() override;
	virtual float GetMaxBrakingDeceleration() const override;
	virtual void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration) override;
	virtual void AddImpulse(FVector Impulse, bool bVelocityChange) override;
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta) override;
	virtual void ApplyImpactPhysicsForces(const FHitResult& Impact, const FVector& ImpactAcceleration, const FVector& ImpactVelocity) override;
	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;
	virtual bool DoJump(bool bReplayingMoves) override;
};
