// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/GrapplingHook/GrapplingComponent.h"
#include "GrappleableComponent.generated.h"



UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UGrappleableComponent : public USceneComponent
{
	GENERATED_BODY()
	
public:

	//eventtype(s)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStartGrapple, const FHitResult&, HitResult);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollisionGrapple, AActor*, GrapplingActor, const FHitResult&, HitResult);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopGrapple);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlternativeAction, APlayerCharacter*, PlayerCharacter);

	//the interp struct for the player when grappling to this object
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGrappleInterpStruct GrappleInterpStructPlayer = FGrappleInterpStruct();

	//the interp struct for the object when the player is grappling to it
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGrappleInterpStruct GrappleInterpStructThis = FGrappleInterpStruct();

	//whether or not to use the grapple interp struct for the player when grappling to this object
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseGrappleInterpStruct = false;

	//whether or not the player can change grapple modes when grappling to this object
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanChangeGrappleMode = true;

	//the multiplier for the grapple reel force applied to this object
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GrappleReelForceMultiplierThis = 1;

	//the multiplier for the grapple reel force applied to the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GrappleReelForceMultiplierPlayer = 1;

	//the max speed to use for the player when grappling to this object (only used if positive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeedPlayer = -1;

	//whether or not to use the alternative action when grappling to this object
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseAlternativeAction = false;

	//whether or not the player is currently grappling to this object
	UPROPERTY(BlueprintReadOnly)
	bool bIsGrappled = false;

	//whether or not we the player should have gravity disabled while grappling to this object
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool NormalMovement = false;

	//constructor
	UGrappleableComponent();

	//event called when the grappling actor starts grappling to this actor
	UPROPERTY(BlueprintAssignable)
	FOnStartGrapple OnStartGrappleEvent;

	//event called when the grappling actor rope collides with this actor
	UPROPERTY(BlueprintAssignable)
	FOnCollisionGrapple OnCollisionGrappleEvent;

	//event called when the grappling actor stops grappling to this actor
	UPROPERTY(BlueprintAssignable)
	FOnStopGrapple OnStopGrappleEvent;

	//event called when the player should do an alternative action while grappling to this object
	UPROPERTY(BlueprintAssignable)
	FOnAlternativeAction AlternativeActionEvent;

	//function called when the grappling actor starts grappling to this actor
	UFUNCTION(BlueprintCallable)
	virtual void OnStartGrapple(const FHitResult& HitResult);

	//function called when the grappling actor rope collides with this actor
	UFUNCTION(BlueprintCallable)
	virtual void OnCollisionGrapple(AActor* OtherActor, const FHitResult& HitResult);

	//function called when the grappling actor stops grappling to this actor
	UFUNCTION(BlueprintCallable)
	virtual void OnStopGrapple();

	//function for whether or not we should use the grapple interp struct of this object
	UFUNCTION(BlueprintCallable)
	virtual bool ShouldUseGrappleInterpStruct() const { return bUseGrappleInterpStruct; }

	//function for getting the grapple interp struct for the object grappling
	UFUNCTION(BlueprintCallable)
	virtual FGrappleInterpStruct GetGrappleInterpStruct() const { return GrappleInterpStructPlayer; }

	//function for whether or not the grappling actor should be able to change grapple modes
	UFUNCTION(BlueprintCallable)
	virtual bool CanChangeGrappleMode() const { return bCanChangeGrappleMode; }
};
