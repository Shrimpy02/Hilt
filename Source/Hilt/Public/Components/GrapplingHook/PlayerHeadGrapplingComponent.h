

#pragma once

#include "CoreMinimal.h"
#include "HeadGrapplingComponent.h"
#include "Components/ActorComponent.h"
#include "PlayerHeadGrapplingComponent.generated.h"

class APlayerCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UPlayerHeadGrapplingComponent : public UHeadGrapplingComponent
{
	GENERATED_BODY()

public:

	//Reference to the player character associated with this component
	UPROPERTY(blueprintreadonly, category = "Core")
	APlayerCharacter* PlayerCharacter = nullptr;

	//the air control you have when grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float AirControl = 2;

	//the friction to use when grappling (only used for lateral friction if bEnableGravity is true)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float Friction = 0.5;

	//the max braking deceleration to use when grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float MaxBrakingDeceleration = 512;

	//whether or not we should enable gravity while grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Gravity")
	bool bEnableGravity = true;

	//the gravity scale multiplier to use when grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Gravity")
	float GravityScale = .5;

	//the total max speed we're using when grappling (used by the movement component)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Speed")
	float MaxSpeed = 8000;

	//the max speed we're using when grappling in the pull mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Speed")
	float MaxSpeedPullMode = 8000;

	//the pull speed multipler (used only when pulling the player character in pull mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Speed")
	float PullSpeedMultiplier = 1.5;

	//the max speed we're using when grappling in the interp mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Speed")
	float MaxSpeedInterpMode = 10000;

	//the acceleration to use when grappling in the interp mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Acceleration")
	float AccelerationInterpMode = 5;

	//the acceleration used for the grapple movement input (in the player movement component)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Acceleration")
	float MaxAcceleration = 2048;

	//the max distance the owner can grapple to from where they are aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StartGrapple|CanGrapple")
	float MaxGrappleDistance = 9000;

	//the distance of the trace/sweep to check if the owner can grapple to something
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StartGrapple|CanGrapple")
	float GrappleCheckDistance = 18000;

	//the amount of wiggle room to give the can grapple check (used to filter out false positives like the camera colliding with the surface the owner is standing on)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StartGrapple|CanGrapple")
	float GrappleCheckWiggleRoom = 1000;

	//the collision channel to use for the grapple check
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StartGrapple|CanGrapple")
	TEnumAsByte<ECollisionChannel> GrappleCheckChannel = ECC_Visibility;

	//the radius used for the aim assist sweep
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StartGrapple|AimAssist")
	float GrappleSphereRadius = 500;

	//constructor(s)
	UPlayerHeadGrapplingComponent();

	//override(s)
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual bool IsGrappling() const override;
	virtual void StartGrapple(const FHitResult& HitResult) override;
	virtual void StopGrapple(bool CallBlueprintEvent = true) override;

	//helper function(s)
private:

	//helper function for the aim assist
	FHitResult GetAimAssistHit(FVector StartLocation, FVector EndLocation) const;

public:

	//function to get the remaining grapple distance
	UFUNCTION(BlueprintCallable)
	float GetRemainingGrappleDistance() const;

	//returns a hit result that can be used to start grappling or an invalid hit result
	UFUNCTION(BlueprintCallable)
	FHitResult GetGrappleHit(float CheckDistance, bool UseAimAssist) const;

	//shorthand function to get whether or not the player can grapple to whatever they're looking at
	UFUNCTION(BlueprintCallable)
	bool PlayerCanGrapple() const;
};
