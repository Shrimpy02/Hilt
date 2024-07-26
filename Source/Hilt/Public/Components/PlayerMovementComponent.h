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

	//event declaration(s)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerImpulse, FVector, Impulse, bool, bVelocityChange);

	//reference to the player as a PlayerCharacter
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	APlayerCharacter* PlayerPawn = nullptr;

	//the max movement speed when falling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling")
	float MaxFallSpeed = 2000;

	//the minimum speed to launch the character off of a collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	float MinCollisionLaunchSpeed = 2000;

	//the maximum speed to launch the character off of a collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	float MaxCollisionLaunchSpeed = 4000;

	//the dot product to use for what is considered a head on collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	float HeadOnCollisionDot = 0.3f;

	//the float curve to use when applying the collision launch speed based on the speed of the player (0 = min speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* CollisionLaunchSpeedCurve = nullptr;

	////the float curve to use for max acceleration when walking based on the speed of the player (0 = min speed, 1 = max speed)
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	//UCurveFloat* MaxWalkingAccelerationCurve = nullptr;

	//the float curve to use for braking deceleration when sliding based on the speed of the player (0 = min speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* WalkingBrakingFrictionCurve = nullptr;

	//the ground friction to apply when sliding
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sliding")
	UCurveFloat* SlidingGroundFrictionCurve = nullptr;

	//the float curve to use for applying braking deceleration when falling (0 = min speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* FallingBrakingDecelerationCurve = nullptr;

	//the player's current speed limit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SpeedLimit = 4000;

	//the minimum speed before the character will be able to do a super jump
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinSpeedForBoostedJump = 2000;

	//whether or not the player is currently forced to be under the speed limit
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsSpeedLimited = true;

	//the built up excess speed from applying the speed limit
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float ExcessSpeed = 0;

	//the max excess speed that can be built up
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxExcessSpeed = 1000;

	//the degredation rate of the excess speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ExcessSpeedDegredationRate = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	float MaxWalkingAcceleration = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Jumping / Falling")
	float AvoidBunnyJumpTraceDistance = 1000;

	//whether or not the player is currently sliding
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sliding")
	bool bIsSliding = false;

	//the speed to add to the player when starting a slide
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sliding")
	float MinSlideStartSpeed = 1000;

	//the curve for the gravity to apply when sliding based on the dot product of the surface normal and the gravity direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* SlideGravityCurve = nullptr;

	//the curve for the turning rate to use when sliding based on the speed of the player (0 = min speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* SlideTurningRateCurve = nullptr;

	//whether or not the player can super jump
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|SuperJump")
	bool bCanSuperJump = true;

	//the amount of force to apply in the direction the player is looking when jumping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|SuperJump")
	float DirectionalJumpForce = 3000;

	//the amount of boost to apply when boosting a jump
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|SuperJump")
	float JumpBoostAmount = 500;

	//whether the player has gone far enough above the ground to be considered not bunny hopping
	UPROPERTY(BlueprintReadOnly, Category = "Character Movement: Jumping / Falling")
	bool bMightBeBunnyJumping = true;

	//whether or not last jump was a directional jump
	bool bLastJumpWasDirectional = false;

	//the direction of the last directional jump
	FVector LastDirectionalJumpDirection = FVector::UpVector;

	//the current slide speed (from either landing or starting a slide)
	float CurrentSlideSpeed = 0;

	//blueprint event(s)
	UPROPERTY(BlueprintAssignable, Category = "Movement")
	FOnPlayerImpulse OnPlayerImpulse;

	//constructor
	UPlayerMovementComponent();

	//function to apply the speed limit to a velocity (if speed limit is enabled)
	UFUNCTION(BlueprintCallable, Category = "Movement")
	FVector ApplySpeedLimit(const FVector& InVelocity, const float& InDeltaTime);

	//function to start sliding
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSlide();

	//function to stop sliding
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSlide();

	//function to get whether or not the player is currently sliding
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsSliding() const;

	//function to get the direction the player is currently sliding
	UFUNCTION(BlueprintCallable, Category = "Movement")
	FVector GetSlideSurfaceDirection();

	//override functions
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void PhysWalking(float deltaTime, int32 Iterations) override;
	virtual void PerformMovement(float DeltaTime) override;
	virtual FVector NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const override;
	virtual void Launch(FVector const& LaunchVel) override;
	virtual FVector ConsumeInputVector() override;
	virtual float GetMaxBrakingDeceleration() const override;
	virtual void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration) override;
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	virtual bool IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const override;
	virtual float GetGravityZ() const override;
	virtual FVector GetAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration) override;
	virtual void StartFalling(int32 Iterations, float remainingTime, float timeTick, const FVector& Delta, const FVector& subLoc) override;
	virtual void PhysFalling(float deltaTime, int32 Iterations) override;
	virtual void AddImpulse(FVector Impulse, bool bVelocityChange) override;
	static float GetAxisDeltaRotation(float InAxisRotationRate, float DeltaTime);
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta) override;
	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;
	virtual bool DoJump(bool bReplayingMoves) override;
};
