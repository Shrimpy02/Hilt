// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Math/InterpShorthand.h"
#include "GrapplingComponent.generated.h"

//enum for different grappling modes based of player input
UENUM(BlueprintType)
enum EGrapplingMode
{
	AddToVelocity,
	InterpVelocity,
};

//struct for the interp struct to use for the grapple
USTRUCT(BlueprintType)
struct FGrappleInterpStruct
{
	GENERATED_BODY()

	//the pull speed to use
	UPROPERTY(editanywhere, BlueprintReadWrite)
	float PullSpeed = 0.f;

	//the pull acceleration to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PullAccel = 0.f;

	//the interp mode to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EInterpToTargetType> InterpMode = InterpTo;
	EInterpToTargetType InInterpMode;

	//constructor(s)
	FGrappleInterpStruct() = default;
	FGrappleInterpStruct(float InPullSpeed, float InPullAccel, EInterpToTargetType InInterpMode);
};

UCLASS()
class UGrapplingComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:

	//events for the grappling
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStartGrapple, const FHitResult&, HitResult);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopGrapple);

	//the rope component to use
	UPROPERTY(BlueprintReadOnly, Category = "Rope")
	class URopeComponent* RopeComponent = nullptr;

	//the grappleable component to use (if any)
	UPROPERTY(BlueprintReadOnly)
	class UGrappleableComponent* GrappleableComponent = nullptr;

	//start grappling event
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStartGrapple OnStartGrapple;

	//stop grappling event
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStopGrapple OnStopGrapple;

	//whether or not we're grappling
	UPROPERTY(BlueprintReadOnly)
	bool bIsGrappling = false;

	//whether or not we're using debug mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bUseDebugMode = false;

	//the current grapple mode
	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EGrapplingMode> GrappleMode = AddToVelocity;

	//the nowasd grapple interp struct
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	FGrappleInterpStruct DefaultGrappleInterpStruct = FGrappleInterpStruct(10000.0f, 5.f, InterpTo);
	
	//the movement input modifier to use when processing the grapple movement input curve
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GrappleMovementInputModifier = 20;

	//the air control you have when grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GrappleAirControl = 2;

	//whether or not to apply gravity when grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bApplyGravityWhenGrappling = false;

	//the max speed to use when grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GrappleMaxSpeed = 8000;

	//the max distance the Grappling hook can travel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CanGrapple")
	float MaxGrappleDistance = 9000;

	//the max distance to check for when checking if the player can grapple to where they are aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CanGrapple")
	float MaxGrappleCheckDistance = 18000;

	//the amount of wiggle room to give the can grapple check
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CanGrapple")
	float GrappleCheckWiggleRoom = 1000;

	//the float curve to use for calculating the score to give from the grapple (0 = no time, > 0 = time)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling")
	UCurveFloat* GrappleScoreCurve = nullptr;

	//the friction to use when grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling")
	float GrappleFriction = 0.5f;

	//the distance threshold to use when checking if we should stop grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling")
	float GrappleStopDistance = 100;

	//the sphere radius to use when checking when double checking the if you can grapple to where you're aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling")
	float GrappleSphereRadius = 100;

	//the grapple dot product to based of the grapple velocity and the player's velocity
	UPROPERTY(BlueprintReadOnly)
	float GrappleDotProduct = 0;

	//the grapple dot product to based of the grapple velocity and (0, 0, 1)
	UPROPERTY(BlueprintReadOnly)
	float AbsoluteGrappleDotProduct = 0;

	//the grapple direction we're using
	UPROPERTY(BlueprintReadOnly)
	FVector GrappleDirection = FVector::ZeroVector;

	//the time that the last grapple started
	UPROPERTY(BlueprintReadOnly)
	float GrappleStartTime = 0;

	//the time to wait before stopping the score degradation when grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling")
	float GrappleScoreDecayStopDelay = 0.5f;

	//whether or not we can grapple right now
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CanGrapple")
	bool CanGrappleVar = false;

	//the amount of pending score to give from the grapple
	UPROPERTY(BlueprintReadOnly)
	float PendingScore = 0;

	//the current grapple input
	UPROPERTY(BlueprintReadOnly)
	FVector GrappleInput = FVector::ZeroVector;

	//storage for the actor we're grappling to
	UPROPERTY(BlueprintReadOnly)
	AActor* GrappleTarget = nullptr;

	//reference to the player movement component
	UPROPERTY()
	class APlayerCharacter* PlayerCharacter = nullptr;

	//storage for the grapple hit(s) we have
	TArray<FHitResult> GrappleHits;

	//constructor
	UGrapplingComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//start grappling function
	UFUNCTION(BlueprintCallable)
	void StartGrapple(const FHitResult& HitResult);

	//stop grappling function
	UFUNCTION(BlueprintCallable)
	void StopGrapple(bool CallBlueprintEvent = true);

	//function to check if we can grapple and start the grapple if we can
	UFUNCTION(BlueprintCallable)
	void StartGrappleCheck();

	//function to check if we should stop grappling
	UFUNCTION(BlueprintCallable)
	void StopGrappleCheck();

	//function to process the grapple input
	UFUNCTION(BlueprintCallable)
	FVector ProcessGrappleInput(FVector MovementInput);

	//whether or not we should use normal movement
	UFUNCTION(BlueprintCallable)
	bool ShouldUseNormalMovement() const;

	//function to get the current max speed to use when grappling
	UFUNCTION(BlueprintCallable)
	float GetMaxSpeed() const;

private:

	//function to handle the interpolation modes of the grapple
	void DoInterpGrapple(float DeltaTime, FVector& GrappleVelocity, FGrappleInterpStruct GrappleInterpStruct);

	//function to do the grapple trace with a given max distance
	void DoGrappleTrace(float MaxDistance, bool DoSphereTrace);

	//function to check for force modifiers based on the grappleable component of the target we're grappling to
	void CheckTargetForceModifiers(FVector& BaseVel, float DeltaTime) const;

	//function to check for force modifiers based on the grappleable component and the current grappling mode
	void CheckTargetPullSpeedModifiers(float& PullSpeed) const;

	//function to apply the pull force to the player
	void ApplyPullForce(float DeltaTime);

	UFUNCTION()
	void OnGrappleTargetDestroyed(AActor* DestroyedActor);

public:
	/**
	 * Getters
	*/

	//function to get the grapple interp struct to use
	UFUNCTION(BlueprintCallable)
	FGrappleInterpStruct GetGrappleInterpStruct() const;

	//function to get the pull speed to use
	UFUNCTION(BlueprintCallable)
	float GetPullSpeed() const;

	//function to get the grappling mode we're in
	UFUNCTION(BlueprintCallable)
	TEnumAsByte<EGrapplingMode> GetGrappleMode() const;

	//function to get the dot product of the grapple direction and the player's velocity
	UFUNCTION()
	float GetGrappleDotProduct(FVector GrappleVelocity) const;

	//function to get the dot product of the grapple direction and (0, 0, 1)
	UFUNCTION()
	static float GetAbsoluteGrappleDotProduct(FVector GrappleVelocity);

	//function to get whether or not we can grapple in the given direction
	UFUNCTION(BlueprintCallable)
	bool CanGrapple(bool DoSphereTrace);

	//function to get the remaining distance in a direction before we hit something we can grapple to
	UFUNCTION(BlueprintCallable)
	float GetRemainingGrappleDistance();

	/**
	 * Setters
	 **/

	//function to set the grapple mode
	UFUNCTION(BlueprintCallable)
	void SetGrappleMode(TEnumAsByte<EGrapplingMode> NewGrappleMode);
};
