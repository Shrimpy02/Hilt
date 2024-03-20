// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CollisionQueryParams.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GrapplingHook/GrapplingComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PlayerMovementComponent.generated.h"

class UPlayerCameraComponent;
class AGrapplingHookHead;

/**
 * Movement component for the player character that adds grappling
 */
UCLASS(Blueprintable)
class UPlayerMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	//the grappling component of the player
	UPROPERTY(BlueprintReadOnly, Category = "Grappling")
	UGrapplingComponent* GrappleComponent = nullptr;

	//the max movement speed when falling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling")
	float MaxFallSpeed = 2000.f;

	//whether or not we're using terminal velocity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling")
	bool bUseTerminalVelocity = true;

	//the float curve to use for setting the max speed when falling and grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling")
	UCurveFloat* MaxSpeedCurve = nullptr;

	//the maximum difference between the current speed and the max speed where the max speed will be increasing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Falling")
	float MaxSpeedDifference = 1000.f;

	//the current time the player has been in the max speed difference range
	UPROPERTY(BlueprintReadOnly, Category = "Falling")
	float MaxSpeedDifferenceTime = 0.f;

	//the max acceleration to use when grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling|Movement")
	float GrappleMaxAcceleration = 2000.f;

	//the minimum speed to launch the character off of a collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	float MinCollisionLaunchSpeed = 1000.f;

	//the maximum speed to launch the character off of a collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	float MaxCollisionLaunchSpeed = 2000.f;

	//the float curve to use when applying the collision launch speed based on the speed of the player (0 = min speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "collision")
	UCurveFloat* CollisionLaunchSpeedCurve = nullptr;

	//constructor
	UPlayerMovementComponent();

	//override functions
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual FVector NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const override;
	virtual void Launch(FVector const& LaunchVel) override;
	virtual FVector ConsumeInputVector() override;
	virtual bool ShouldRemainVertical() const override;
	virtual bool IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const override;
	virtual float GetGravityZ() const override;

	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta) override;
	virtual void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration) override;

	//function called when the player starts grappling
	UFUNCTION()
	void OnStartGrapple(AActor* OtherActor, const FHitResult& HitResult);

	//function called when the player stops grappling
	UFUNCTION()
	void OnStopGrapple();
};
