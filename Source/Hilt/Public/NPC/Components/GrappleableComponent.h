// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/GrapplingHook/GrapplingComponent.h"
#include "GrappleableComponent.generated.h"



UCLASS()
class UGrappleableComponent : public USceneComponent
{
	GENERATED_BODY()
	
public:

	//the interp struct for the grapple
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGrappleInterpStruct GrappleInterpStruct;

	//constructor
	UGrappleableComponent();

	//function called when the grappling actor starts grappling to this actor
	UFUNCTION(BlueprintCallable)
	virtual void OnStartGrapple(AActor* OtherActor, const FHitResult& HitResult);

	//function called when the grappling actor stops grappling to this actor
	UFUNCTION(BlueprintCallable)
	virtual void OnStopGrapple();

	//function that is called while this actor is being grappled to (this is called every frame while attached to the rope) (returns the actor on the other end of the rope (if there is one))
	UFUNCTION(BlueprintCallable)
	virtual void WhileGrappled(float DeltaTime);

	//function for whether or not we should use the grapple interp struct of this object
	UFUNCTION(BlueprintCallable)
	virtual bool ShouldUseGrappleInterpStruct() const { return true; }

	//function for getting the grapple interp struct for the object grappling
	UFUNCTION(BlueprintCallable)
	virtual FGrappleInterpStruct GetGrappleInterpStruct() const { return GrappleInterpStruct; }

	//function for whether or not the grappling actor should be able to change grapple modes
	UFUNCTION(BlueprintCallable)
	virtual bool CanChangeGrappleMode() const { return true; }
};
