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

/**
 * Movement component for the player character that extends the default character movement component
 */
UCLASS(Blueprintable)
class UPlayerMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	//reference to the player as a PlayerCharacter
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	APlayerCharacter* PlayerPawn = nullptr;

	//the max movement speed when falling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling")
	float MaxFallSpeed = 2000.f;

	//the minimum speed to launch the character off of a collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	float MinCollisionLaunchSpeed = 2000.f;

	//the maximum speed to launch the character off of a collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	float MaxCollisionLaunchSpeed = 4000.f;

	//the dot product to use for what is considered a head on collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	float HeadOnCollisionDot = 0.3f;

	//the float curve to use when applying the collision launch speed based on the speed of the player (0 = min speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* CollisionLaunchSpeedCurve = nullptr;

	//the float curve to use for max acceleration when walking based on the speed of the player (0 = min speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* MaxWalkingAccelerationCurve = nullptr;

	//the float curve to use for the braking friction based on the speed of the player (0 = min speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* BrakingFrictionCurve = nullptr;

	//the player's current speed limit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SpeedLimit = 4000.f;

	//the minimum speed before the character will be able to do a super jump
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinSpeedForBoostedJump = 2000.f;

	//whether or not the player is currently forced to be under the speed limit
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsSpeedLimited = true;

	//the max acceleration when reversing walking direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxReverseWalkingAcceleration = 9999.f;

	//the built up excess speed from applying the speed limit
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float ExcessSpeed = 0.f;

	//the max excess speed that can be built up
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxExcessSpeed = 1000.f;

	//the degredation rate of the excess speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ExcessSpeedDegredationRate = 10.f;

	//the amount of excess speed needed to stop applying the speed limit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ExcessSpeedStopLimit = 10.f;

	//the amount of force to apply in the direction the player is looking when jumping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuperJump")
	float DirectionalJumpForce = 3000.f;

	//the amount of boost to give to the character while a directional jump is providing force
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuperJump")
	float DirectionalJumpGlideForce = 500.f;

	//the amount of boost to apply when boosting a jump
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SuperJump")
	float JumpBoostAmount = 500.f;

	//whether or not last jump was a directional jump
	bool bLastJumpWasDirectional = false;

	//the direction of the last directional jump
	FVector LastDirectionalJumpDirection = FVector::UpVector;

	//constructor
	UPlayerMovementComponent();

	//function to apply the speed limit to a velocity (if speed limit is enabled)
	UFUNCTION(BlueprintCallable, Category = "Movement")
	FVector ApplySpeedLimit(const FVector& InVelocity, const float& InDeltaTime);

	//override functions
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual FVector NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const override;
	virtual void Launch(FVector const& LaunchVel) override;
	virtual FVector ConsumeInputVector() override;
	//virtual bool ShouldRemainVertical() const override;
	virtual bool IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const override;
	virtual float GetGravityZ() const override;

	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta) override;
	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;
	//virtual void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration) override;
	virtual bool DoJump(bool bReplayingMoves) override;
};
