// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "RopeComponent.generated.h"

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

	//constructor(s)
	FRopePoint();
	explicit FRopePoint(const FHitResult& HitResult);
	explicit FRopePoint(AActor* OtherActor, const FVector& Location);

	//function to get the location of the rope point in world space
	FVector GetWL() const;
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

private:
	//whether or not the rope is currently active
	UPROPERTY(BlueprintReadOnly, Category = "Rope", meta=(AllowPrivateAccess))
	bool bIsRopeActive = false;

public:

	//constructor
	URopeComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void DestroyComponent(bool bPromoteChildren) override;

	//function for switching the rope niagara system
	UFUNCTION(BlueprintCallable, Category = "Rope")
	void SetNiagaraSystem(UNiagaraSystem* NewSystem);

	//function to get the collision query params used for the rope's collision checks
	FCollisionQueryParams GetCollisionParams();

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
