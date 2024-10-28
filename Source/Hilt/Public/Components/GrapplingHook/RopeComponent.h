// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "RopeComponent.generated.h"

//struct for rope points
USTRUCT(BlueprintType)
struct FRopePoint
{
	GENERATED_BODY()

	//the attached actor for the rope point
	UPROPERTY(BlueprintReadOnly)
	AActor* AttachedActor = nullptr;

	//the component to use for the rope point's location
	UPROPERTY(BlueprintReadOnly)
	USceneComponent* Component = nullptr;

	//the relative location of the rope point to the attached actor (if not using a component's location)
	UPROPERTY(BlueprintReadOnly)
	FVector Location = FVector::ZeroVector;

	//the velocity of the rope point
	UPROPERTY(BlueprintReadOnly)
	FVector Velocity = FVector::ZeroVector;

	//the acceleration of the rope point
	UPROPERTY(BlueprintReadOnly)
	FVector Acceleration = FVector::ZeroVector;

	//whether or not this rope point is a collision point
	UPROPERTY(BlueprintReadOnly)
	bool bIsCollisionPoint = false;

	//whether or not to use world space for the location of the rope point
	UPROPERTY(BlueprintReadOnly)
	bool bUseWorldSpace = false;
	
	////older locations of the rope point for verlet integration
	//UPROPERTY(BlueprintReadOnly)
	//TArray<FVector> OldLocations;

	////the deltatimes that were used for the old locations
	//UPROPERTY(BlueprintReadOnly)
	//TArray<float> OldTimesteps;

	////the mass of this rope point
	//UPROPERTY(BlueprintReadOnly)
	//float Mass = 1;

	//constructor(s)
	FRopePoint();
	explicit FRopePoint(FVector InLocation, bool bInUseWorldSpace = true);
	explicit FRopePoint(const FHitResult& HitResult);
	explicit FRopePoint(AActor* InOtherActor, const FVector& InLocation);

	//function to get the location of the rope point in world space
	FVector GetWL() const;

	//function to set the location of the rope point in world space (if using relative location, will set the location of the attached actor)
	void SetWL(const FVector& NewLocation);
};

//struct for constraints between rope points
USTRUCT(BlueprintType)
struct FVerletConstraint
{
	GENERATED_BODY()

	//start point of the constraint
	struct FRopePoint* StartPoint = nullptr;

	//end point of the constraint
	struct FRopePoint* EndPoint = nullptr;

	//the compensation to apply to the first point of the constraint
	UPROPERTY(BlueprintReadOnly)
	float Compensation1 = 0.5;

	//the compensation to apply to the second point of the constraint
	UPROPERTY(BlueprintReadOnly)
	float Compensation2 = 0.5;

	//the length of the constraint
	UPROPERTY(BlueprintReadOnly)
	float Distance = 0.f;

	//constructor(s)
	FVerletConstraint();
	explicit FVerletConstraint(FRopePoint* InStartPoint, FRopePoint* InEndPoint, float InCompensation1 = 0.5, float InCompensation2 = 0.5, float InDistance = 0);

	//function to get the first point of the constraint
	FVector GetStartPoint() const;

	//function to get the second point of the constraint
	FVector GetEndPoint() const;

	//function to get the distance between the two points of the constraint
	float GetDistance() const;

	//function to set the first point of the constraint
	void SetStartPoint(const FVector& NewStartPoint) const;

	//function to set the second point of the constraint
	void SetEndPoint(const FVector& NewEndPoint) const;

	//function to set the distance between the two points of the constraint
	void SetDistance(float NewDistance);
};

UCLASS()
class URopeComponent : public USceneComponent
{
	GENERATED_BODY()
	
public:

	//list of classes that the rope should ignore when checking for collisions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope")
	TArray<TSubclassOf<AActor>> IgnoredClasses;

	//the possible grappleable component for the end of the rope
	UPROPERTY(BlueprintReadOnly)
	class UGrappleableComponent* GrappleableComponent = nullptr;

	//whether or not to use debug drawing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope|Debug")
	bool bUseDebugDrawing = false;

	//the radius of the rope
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Rope")
	float RopeRadius = 10.f;

	//the Niagara system used to render the rope
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope|Rendering")
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	//the name of the user parameter for the end of the Niagara ribbons
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope|Rendering")
	FName RibbonEndParameterName = "HookEnd";

	//array of niagara components used to render the rope
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rope|Rendering")
	TArray<UNiagaraComponent*> NiagaraComponents;

	//the minimum spacing between new and old rope points in the infinite length rope mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Rope")
	float MinCollisionPointSpacing = 20.f;

	//the collision channel to use for the collision checks of the rope
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_Visibility;

	//array of rope points for the rope
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rope", meta = (ShowOnlyInnerProperties))
	TArray<FRopePoint> RopePoints;

	//whether or not to use verlet integration for the rope
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	bool bUseVerletIntegration = false;

	//array of constraints for the verlet integration rope points
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Verlet Integration", meta = (ShowOnlyInnerProperties))
	TArray<FVerletConstraint> Constraints;

	//the number of verlet rope points to use between each 2 rope points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	int32 NumVerletPoints = 250;

	////how many times to perform the verlet integration per frame
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	//int32 NumVerletIterations = 1;

	//how many times to perform the constraint enforcement per frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	int32 NumConstraintIterations = 25;

	////how many old locations to store for each rope point
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	//int32 NumOldLocations = 2;

	//the float curve for the compensation1 of the constraints
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	UCurveFloat* ConstraintCompensation1Curve = nullptr;

	//the float curve for the compensation2 of the constraints
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	UCurveFloat* ConstraintCompensation2Curve = nullptr;

	//the damping factor for the verlet integration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	float Damping = 0.85;

	//the stiffness of the constraints
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	float Stiffness = 1;

	//the gravity to apply to the rope points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	float VerletGravityFactor = 4;

	//the drag to apply to the rope points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	float RopeDrag = 0.1;

	//the mass of each rope point
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	float RopeMass = 1;

	//array of collision points for the rope
	TArray<FRopePoint*> CollisionPoints;

private:
	//whether or not the rope is currently active
	UPROPERTY(BlueprintReadOnly, Category = "Rope", meta=(AllowPrivateAccess))
	bool bIsRopeActive = false;

	//storage for the player character
	UPROPERTY(BlueprintReadOnly, Category = "Rope", meta = (AllowPrivateAccess))
	class APlayerCharacter* PlayerCharacter = nullptr;

public:

	//constructor
	URopeComponent();

	//overrides
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void DestroyComponent(bool bPromoteChildren) override;

	//function to enforce the constraints of the rope
	void EnforceConstraints();

	//function to check for collisions with the rope when verlet integration is used and update the rope points accordingly
	bool CheckForCollisions(const FVector& Start, const FVector& End, FRopePoint& Point) const;
	bool CheckForCollisions(const FVerletConstraint& Constraint, const FVector& InNewStartPos1, const FVector& InNewStartPos2) const;
	bool CheckForCollisions(FRopePoint& Point, const FVector& InNewPosition, const FVector& OldPosition, const FVector& InVelocity, const FVector& InAcceleration) const;

	//function to do all verlet integration steps for this frame
	void VerletIntegration(float DeltaTime);

	//function to apply forces to the rope point (used for velocity-verlet integration)
	FVector CalculateAccel(const FRopePoint& RopePoint) const;

	//function for switching the rope niagara system
	UFUNCTION(BlueprintCallable, Category = "Rope")
	void SetNiagaraSystem(UNiagaraSystem* NewSystem);

	//function to get the collision query params used for the rope's collision checks
	FCollisionQueryParams GetCollisionParams() const;

	//traces along the collision points and removes unnecessary collision points
	void CheckCollisionPoints();

	//spawns a new niagara system for a rope point at the given index in the rope points array, pointing towards the next point in the array (not called for the last point in the array)
	void SpawnNiagaraSystem(int Index);

	//renders the rope using the niagara system
	void RenderRope();

	//function to deactivate the rope
	UFUNCTION()
	void DeactivateRope();

	//function to activate the rope
	UFUNCTION()
	void ActivateRope(const FHitResult& HitResult);

	/**
	 * Getters
	*/

	//function to get the direction of the rope from a given t value and direction (forwards or backwards)
	UFUNCTION(BlueprintCallable, Category = "Rope")
	FVector GetRopeDirection() const;

	//function to get the length of the rope
	UFUNCTION(BlueprintCallable, Category = "Rope")
	float GetRopeLength() const;

	//function to get the end of the rope
	UFUNCTION(BlueprintCallable, Category = "Rope")
	FVector GetRopeEnd() const;

	//function to get the second rope point
	UFUNCTION(BlueprintCallable, Category = "Rope")
	FVector GetSecondRopePoint() const;
};
