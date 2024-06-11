// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "RopeComponent.generated.h"

UCLASS()
class URopeComponent : public USceneComponent
{
	GENERATED_BODY()
	
public:

	//the possible sphere sweep hit for the end of the rope
	UPROPERTY(BlueprintReadOnly)
	FHitResult StartHit = FHitResult();

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
	FName RibbonEndParameterName = "RopeEnd";

	//whether or not to use the rope radius as the ribbon width
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope|Rendering")
	bool UseRopeRadiusAsRibbonWidth = true;

	//the name of the user parameter for the width of the Niagara ribbons
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope|Rendering", meta = (EditCondition = "UseRopeRadiusAsRibbonWidth == false", EditConditionHides))
	FName RibbonWidthParameterName = "RopeWidth";

	//the width of the ribbon used to render the rope(only used if UseRopeRadiusAsRibbonWidth is false)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope|Rendering", meta = (EditCondition = "UseRopeRadiusAsRibbonWidth == false", EditConditionHides))
	float RibbonWidth = 10.f;

	//array of niagara components used to render the rope
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rope|Rendering")
	TArray<UNiagaraComponent*> NiagaraComponents;

	//the minimum spacing between new and old rope points in the infinite length rope mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Rope|InfiniteLength")
	float MinCollisionPointSpacing = 20.f;

	//array of rope points used when the rope is infinite length
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rope", meta = (ShowOnlyInnerProperties))
	TArray<FVector> RopePoints = {FVector(0, 0, 0), FVector(0, 0, 0)};

	//the tick behaviour to use for the niagara components
	ENiagaraTickBehavior TickBehavior = ENiagaraTickBehavior::UseComponentTickGroup;

	//the tick group to use for the niagara components
	TEnumAsByte<ETickingGroup> TickGroup = TG_LastDemotable;

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

	//traces along the collision points and removes unnecessary collision points
	void CheckCollisionPoints();

	//sets the collision points for the rope to the current location of the actors we're attached to
	void SetAttachedRopePointPositions();

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
public:
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
