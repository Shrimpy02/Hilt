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
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStartGrapple, AActor*, OtherActor, const FHitResult&, HitResult);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopGrapple);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWhileGrappled, float, DeltaTime);

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

	//while grappling event
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FWhileGrappled WhileGrappled;

	//whether or not we're grappling
	UPROPERTY(BlueprintReadOnly)
	bool bIsGrappling = false;

	//the current grapple mode
	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EGrapplingMode> GrappleMode = AddToVelocity;

	//the grappling speed in add velocity mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float WasdPullSpeed = 22500;

	//the nowasd grapple interp struct
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	FGrappleInterpStruct NoWasdGrappleInterpStruct = FGrappleInterpStruct(10000.0f, 5.f, InterpTo);

	//whether or not to disable gravity when grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
	bool bDisableGravityWhenGrappling = true;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collisions")
	ECollisionShape::Type CanGrappleCollisionShape = ECollisionShape::Sphere;
	
	//the movement input modifier to use when processing the grapple movement input curve
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GrappleMovementInputModifier = 20;

	//whether or not to apply gravity when grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bApplyGravityWhenGrappling = true;

	//the max speed to use when grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GrappleMaxSpeed = 4000.f;

	//the max distance the Grappling hook can travel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CanGrapple")
	float MaxGrappleDistance = 17000.f;

	//the max distance to check for when checking if the player can grapple to where they are aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CanGrapple")
	float MaxGrappleCheckDistance = 18000.f;

	//the trace channel to use when checking if the player can grapple to where they are aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CanGrapple");
	TEnumAsByte<ECollisionChannel> CanGrappleTraceChannel = ECC_Visibility;

	//the float curve to use when applying the grapple velocity using the dot product of the character's velocity and the velocity that was added from grappling last frame (-1 = opposite direction, 0 = perpendicular(90 degrees), 1 = same direction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling")
	UCurveFloat* GrappleAngleCurve = nullptr;

	//the float curve to use when applying the grapple velocity using the rope length divided by the max grapple distance (1 = max distance, 0 = 0 distance, clamped to 0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling")
	UCurveFloat* GrappleDistanceCurve = nullptr;

	//the float curve to use when applying the grapple wasd movement using the dot product of the character's up vector (so a 90 degree angle off of the the vector pointing to the grappling point) and the velocity that will be added from this input (-1 = opposite direction, 0 = perpendicular(90 degrees), 1 = same direction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling|Movement")
	UCurveFloat* GrappleMovementAngleInputCurve = nullptr;

	//the float curve to use when applying the grapple wasd movement using the rope length divided by the max grapple distance (1 = max distance, 0 = 0 distance, clamped to 0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling|Movement")
	UCurveFloat* GrappleMovementDistanceInputCurve = nullptr;

	////the float curve to use for modifying the pull force based on the number of collisions the grappling rope has (starts at 2 because its counting the player and the grapple point)
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling")
	//UCurveFloat* GrappleCollisionPointsCurve = nullptr;

	////the float curve to use for modifying the pull force based on how close the player is to max speed and how close the grapple direction is to the player's velocity direction
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling")
	//UCurveFloat* GrappleSpeedAndDirectionForceCurve = nullptr;

	////the float curve to use for modifying the pull force based on the relative length of the last segment of the rope
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling")
	//UCurveFloat* GrappleLastSegmentLengthCurve = nullptr;

	//the grapple dot product to based of the grapple velocity and the player's velocity
	UPROPERTY(BlueprintReadOnly)
	float GrappleDotProduct = 0.f;

	//the grapple dot product to based of the grapple velocity and (0, 0, 1)
	UPROPERTY(BlueprintReadOnly)
	float AbsoluteGrappleDotProduct = 0.f;

	//the grapple direction we're using
	UPROPERTY(BlueprintReadOnly)
	FVector GrappleDirection = FVector::ZeroVector;

	//whether or not we can grapple right now
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CanGrapple")
	bool CanGrappleVar = false;

	//the current grapple input
	UPROPERTY(BlueprintReadOnly)
	FVector GrappleInput = FVector::ZeroVector;

	//reference to the player movement component
	UPROPERTY()
	class UPlayerMovementComponent* PlayerMovementComponent = nullptr;

	//constructor
	UGrapplingComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//start grappling function
	UFUNCTION(BlueprintCallable)
	void StartGrapple(AActor* OtherActor, const FHitResult& HitResult);

	//stop grappling function
	UFUNCTION(BlueprintCallable)
	void StopGrapple();

	//function to check if we can grapple and start the grapple if we can
	UFUNCTION(BlueprintCallable)
	void StartGrappleCheck();

	//function to process the grapple input
	UFUNCTION(BlueprintCallable)
	FVector ProcessGrappleInput(FVector MovementInput);

private:

	//function to handle the interpolation modes of the grapple
	void DoInterpGrapple(float DeltaTime, FVector& GrappleVelocity, FGrappleInterpStruct GrappleInterpStruct);

	//function to do the grapple trace with a given max distance
	void DoGrappleTrace(FHitResult& GrappleHit, float MaxDistance) const;

	//function to apply the pull force to the player
	void ApplyPullForce(float DeltaTime);

public:
	/**
	 * Getters
	*/

	//function to get the direction of the grapple
	UFUNCTION(BlueprintCallable)
	FVector GetGrappleDirection() const;

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
	bool CanGrapple() const;

	//function to get the remaining distance in a direction before we hit something we can grapple to
	UFUNCTION(BlueprintCallable)
	float GetRemainingGrappleDistance() const;

	/**
	 * Setters
	 **/

	//function to set the grapple mode
	UFUNCTION(BlueprintCallable)
	void SetGrappleMode(TEnumAsByte<EGrapplingMode> NewGrappleMode);
};
