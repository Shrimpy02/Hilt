

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeadGrapplingComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UHeadGrapplingComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	//events for the grappling
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStartGrapple, const FHitResult&, HitResult);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopGrapple);

	//start grappling event
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStartGrapple OnStartGrapple;

	//stop grappling event
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStopGrapple OnStopGrapple;

	//the rope component that the owner has
	UPROPERTY(BlueprintReadOnly, Category = "Core")
	class URopeComponent* RopeComponent = nullptr;

	//the grappleable component that the object we're grappling to has (if any)
	UPROPERTY(BlueprintReadOnly, Category = "Grappleable")
	class UGrappleableComponent* GrappleableComponent = nullptr;

	//storage for the actor we're grappling to
	UPROPERTY(BlueprintReadOnly, Category = "Core")
	AActor* GrappleTarget = nullptr;

	//the time that the last grapple started
	UPROPERTY(BlueprintReadOnly, Category = "Core")
	float GrappleStartTime = 0;

	//the distance threshold to use when checking if we should stop grappling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StopGrapple|Autostop")
	float GrappleStopDistance = 500;

	//constructor(s)
	UHeadGrapplingComponent();

	//override(s)
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//whether or not we are grappling
	UFUNCTION(BlueprintCallable)
	virtual bool IsGrappling() const;

	//function to get the location we're currently grappling to
	UFUNCTION(BlueprintCallable)
	virtual FVector GetEndLocation() const;

	//function to get the direction we're currently grappling in (normalized)
	UFUNCTION(BlueprintCallable)
	virtual FVector GetGrappleDirection() const;

	//starts the grappling process
	UFUNCTION(BlueprintCallable)
	virtual void StartGrapple(const FHitResult& HitResult);

	//stops the grappling process
	UFUNCTION(BlueprintCallable)
	virtual void StopGrapple(bool CallBlueprintEvent = true);

	//function to check if we should autostop grappling
	UFUNCTION(BlueprintCallable)
	virtual bool ShouldAutostopGrapple() const;

	//helper function(s)
protected:

	//helper function to get the parameters for the grapple check
	FCollisionQueryParams GetCollisionParams() const;

	//functions used for binding to events
public:

	UFUNCTION()
	void OnGrappleTargetDestroyed(AActor* DestroyedActor);
	
};
