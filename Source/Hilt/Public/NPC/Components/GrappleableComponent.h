// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/GrapplingHook/PlayerGrapplingComponent.h"
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

	//whether or not to use the alternative action when grappling to this object
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseAlternativeAction = false;

	//whether or not the player is currently grappling to this object
	UPROPERTY(BlueprintReadOnly)
	bool bIsGrappled = false;

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
};
