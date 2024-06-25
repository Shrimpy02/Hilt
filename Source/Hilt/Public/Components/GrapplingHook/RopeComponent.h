// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "RopeComponent.generated.h"

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
};

//USTRUCT(BlueprintType)
//struct FVerletRopePoint
//{
//	GENERATED_BODY()
//
//	//the current location of the rope point
//	UPROPERTY(BlueprintReadOnly)
//	FVector Position = FVector::ZeroVector;
//
//	//the previous location of the rope point in world space
//	UPROPERTY(BlueprintReadOnly, Category = "Verlet Integration")
//	FVector OldPosition = FVector::ZeroVector;
//
//	//the mass of this rope point
//	UPROPERTY(BlueprintReadOnly, Category = "Verlet Integration")
//	float Mass = 1.f;
//
//	//constructor(s)
//	FVerletRopePoint();
//	explicit FVerletRopePoint(const FVector& InPosition, float InMass = 1.f);
//};

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
	FVector RelativeLocation = FVector::ZeroVector;

	//whether or not this rope point is a collision point
	UPROPERTY(BlueprintReadOnly)
	bool bIsCollisionPoint = false;

	//whether or not to use world space for the location of the rope point
	UPROPERTY(BlueprintReadOnly)
	bool bUseWorldSpace = false;
	
	//the previous location of the rope point in world space
	UPROPERTY(BlueprintReadOnly, Category = "Verlet Integration")
	FVector OldPosition = FVector::ZeroVector;

	//the mass of this rope point
	UPROPERTY(BlueprintReadOnly, Category = "Verlet Integration")
	float Mass = 1;

	//constructor(s)
	FRopePoint();
	explicit FRopePoint(FVector InLocation, bool bInUseWorldSpace = true, float InMass = 1.f);
	explicit FRopePoint(const FHitResult& HitResult);
	explicit FRopePoint(AActor* OtherActor, const FVector& Location);

	//function to get the location of the rope point in world space
	FVector GetWL() const;

	//function to set the location of the rope point in world space (if using relative location, will set the location of the attached actor)
	void SetWL(const FVector& NewLocation);
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

	//array of constraints for the verlet integration rope points
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Verlet Integration", meta = (ShowOnlyInnerProperties))
	TArray<FVerletConstraint> Constraints;

	//the number of verlet rope points to use between each 2 rope points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	int32 NumVerletPoints = 5;

	//how many times to perform the verlet integration per frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	int32 NumVerletIterations = 5;

	//the float curve for the initial placement of the verlet rope points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	UCurveFloat* InitialVerletPlacementCurve = nullptr;

	//the float curve for the compensation1 of the constraints
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	UCurveFloat* ConstraintCompensation1Curve = nullptr;

	//the float curve for the compensation2 of the constraints
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	UCurveFloat* ConstraintCompensation2Curve = nullptr;

	//the damping factor for the verlet integration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Verlet Integration")
	float Damping = 0.99f;


private:
	//whether or not the rope is currently active
	UPROPERTY(BlueprintReadOnly, Category = "Rope", meta=(AllowPrivateAccess))
	bool bIsRopeActive = false;

public:

	//constructor
	URopeComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void DestroyComponent(bool bPromoteChildren) override;

	//function to do a single verlet integration step with constraints
	void VerletIntegrationStep(float DeltaTime);

	//function to enforce the constraints of the rope
	void EnforceConstraints();

	//function to do all verlet integration steps for this frame
	void VerletIntegration(float DeltaTime);

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
	void ActivateRope(AActor* OtherActor, const FHitResult& HitResult);

	/**
	 * Getters
	*/

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
