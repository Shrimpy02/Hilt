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
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerStartSlide);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerStopSlide);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerStartDive);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerStopDive);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerSuperJump);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerNormalJump);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerStartFall, const FVector&, PreviousFloorImpactNormal, const FVector&, PreviousFloorContactNormal, const FVector&, PreviousLocation);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerHitObjective);

	//reference to the player as a PlayerCharacter
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	APlayerCharacter* PlayerPawn = nullptr;

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

	//the amount of score to subtract when you collide
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	float CollisionScoreLoss = 1;

	//the float curve to use when applying the collision launch speed based on the speed of the player (0 = min speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* CollisionLaunchSpeedCurve = nullptr;

	//the float curve to use for braking deceleration when sliding based on the speed of the player (0 = min speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* WalkingBrakingFrictionCurve = nullptr;

	//the float curve to use for adding score when stopping a slide based on the time spend sliding
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* SlideScoreCurve = nullptr;

	//the float curve to use for processing landing based on the dot product of the player's velocity and the surface normal (-1 = opposite, 1 = aligned)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* SlideLandingDotCurve = nullptr;

	//the pending score for the slide
	UPROPERTY(BlueprintReadOnly, Category = "Movement|Sliding")
	float PendingSlideScore = 0;

	//the ground friction to apply when sliding
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sliding")
	UCurveFloat* SlidingGroundFrictionCurve = nullptr;

	//the float curve to use for applying braking deceleration when falling (0 = min speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* FallingBrakingDecelerationCurve = nullptr;

	//the float curve to use for the multiplier to apply to the player's speed when slide jumping (0 = min speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* SlideJumpSpeedCurve = nullptr;

	//the float curve to use for the multiplier to apply to the player's speed when slide jumping (1 = aligned with velocity, -1 = opposite of velocity)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* SlideJumpDirectionCurve = nullptr;

	////the float curve for the slide fall turn rate limiter (0 = no time spent falling)
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	//UCurveFloat* SlideFallTurnRateCurve = nullptr;

	//the float curve for the wasd movement multiplier when diving (0 = no speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* DiveWasdCurve = nullptr;

	//the float curve for the max wasd speed (0 just started diving)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* DiveMaxWasdSpeedCurve = nullptr;

	//the float curve for the terminal velocity when diving (0 = just started diving)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* DiveTerminalVelocityCurve = nullptr;

	////the float curve for the dive movement multiplier when diving (-1 = moving upwards, 1 = moving downwards)
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	//UCurveFloat* DiveMovementDirectionCurve = nullptr;

	//the float curve for the terminal velocity after diving (0 = just stopped diving)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* AfterDiveTerminalVelocityCurve = nullptr;

	//the player's current speed limit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SpeedLimit = 4000;

	//whether or not the player is currently forced to be under the speed limit
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsSpeedLimited = true;

	//the built up excess speed from applying the speed limit
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	mutable float ExcessSpeed = 0;

	//the max excess speed that can be built up
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxExcessSpeed = 1000;

	//the degredation rate of the excess speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ExcessSpeedDegredationRate = 10;

	//the max acceleration to apply when walking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	float MaxWalkingAcceleration = 1000;

	//the distance to trace for to check if the player is bunny hopping (in which case we don't want them to have falling speed or other physics applied from falling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Jumping / Falling")
	float AvoidBunnyJumpTraceDistance = 1000;

	//whether or not the player is currently sliding
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sliding")
	bool bIsSliding = false;

	//whether or not we're slide falling
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sliding")
	bool bIsSlideFalling = false;

	//the amount of time to wait before stopping the slide falling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sliding")
	float SlideFallStopDelay = 0.5;

	//how often we should bank the player's score when sliding (0 = never, 1 = every second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sliding")
	float SlideScoreBankRate = 1;

	//the speed to add to the player when starting a slide
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sliding")
	float MinSlideStartSpeed = 1000;

	//the amount of time to wait before stopping the score degradation when sliding
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sliding")
	float SlideScoreDecayStopDelay = 0.5;

	//the slide jump force multiplier
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sliding|Jumping")
	float SlideJumpForceMultiplier = 1;

	//the slide jump time
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Sliding|Jumping")
	float SlideJumpTime = 0.5;

	//whether the player has gone far enough above the ground to be considered not bunny hopping
	UPROPERTY(BlueprintReadOnly, Category = "Character Movement: Jumping / Falling")
	bool bMightBeBunnyJumping = true;

	//whether or not the player is currently diving
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Diving")
	bool bIsDiving = false;

	//the gravity scale to use when diving
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Diving")
	float DiveGravityScaleMultiplier = 8;

	//the direction of the last directional jump
	FVector LastSuperJumpDirection = FVector::UpVector;

	//the amount of speed gained from sliding since the last slide start
	float SlideSpeedGained = 1000;

	//the current slide speed (from either landing or starting a slide)
	float CurrentSlideSpeed = 0;

	//storage for the time we started sliding
	float SlideStartTime = 0;

	//whether or not we're slide jumping
	bool bIsSlideJumping = false;

	//timer handle for banking slide score
	FTimerHandle SlideScoreBankTimer;

	//the gravity scale used at begin play
	float DefaultGravityScale = 1;

	//the time when the slide fall started
	float SlideFallStartTime = 0;

	//the time when the player started diving
	float DiveStartTime = 0;

	//the time when the player stopped diving
	float DiveStopTime = 0;

	//blueprint event(s)
	UPROPERTY(BlueprintAssignable, Category = "Movement")
	FOnPlayerImpulse OnPlayerImpulse;

	UPROPERTY(BlueprintAssignable, Category = "Movement")
	FOnPlayerStartSlide OnPlayerStartSlide;

	UPROPERTY(BlueprintAssignable, Category = "Movement")
	FOnPlayerStopSlide OnPlayerStopSlide;

	UPROPERTY(BlueprintAssignable, Category = "Movement")
	FOnPlayerStartDive OnPlayerStartDive;

	UPROPERTY(BlueprintAssignable, Category = "Movement")
	FOnPlayerStopDive OnPlayerStopDive;

	UPROPERTY(BlueprintAssignable, Category = "Movement")
	FOnPlayerSuperJump OnPlayerSLideJump;

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
	FVector ApplySpeedLimit(const FVector& InVelocity, const float& InDeltaTime, bool AddToExcessSpeed = true) const;

	//function to get the current speed limit (taking into account the current score multiplier)
	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetCurrentSpeedLimit() const;

	//function to start sliding
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSlide();

	//function to stop sliding
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSlide();

	//function to get whether or not the player is currently sliding
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsSliding() const;

	//function to bank the pending score from sliding
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void BankSlideScore();

	//function to get the direction the player is currently sliding
	UFUNCTION(BlueprintCallable, Category = "Movement")
	FVector GetSlideSurfaceDirection();

	//function to start diving
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartDive();

	//function to stop diving
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopDive();

	//function to get whether or not the player is currently diving
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsDiving() const;

	//override functions
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void PhysWalking(float deltaTime, int32 Iterations) override;
	virtual void PhysFalling(float deltaTime, int32 Iterations) override;
	virtual bool IsWalkable(const FHitResult& Hit) const override;
	virtual void PerformMovement(float DeltaTime) override;
	virtual void HandleWalkingOffLedge(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta) override;
	virtual FVector NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const override;
	virtual float GetGravityZ() const override;
	virtual FVector GetAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration) override;
	virtual void StartFalling(int32 Iterations, float remainingTime, float timeTick, const FVector& Delta, const FVector& subLoc) override;
	virtual FVector ConsumeInputVector() override;
	virtual float GetMaxBrakingDeceleration() const override;
	virtual void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration) override;
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	virtual bool IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const override;
	virtual void AddImpulse(FVector Impulse, bool bVelocityChange) override;
	static float GetAxisDeltaRotation(float InAxisRotationRate, float DeltaTime);
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta) override;
	virtual void ApplyImpactPhysicsForces(const FHitResult& Impact, const FVector& ImpactAcceleration, const FVector& ImpactVelocity) override;
	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;
	virtual bool DoJump(bool bReplayingMoves) override;
};
