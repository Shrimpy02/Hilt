// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiveComponent.generated.h"

UCLASS()
class UDiveComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:

	//events for Diving
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartDive);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopDive);

	//start Diving event
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStartDive OnStartDive;

	//stop Diving event
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStopDive OnStopDive;

	//the float curve for the wasd movement multiplier when diving (0 = no speed, 1 = max speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* WasdCurve = nullptr;

	//the float curve for the max wasd speed (0 just started diving)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* MaxSpeedCurve = nullptr;

	//the float curve for the terminal velocity when diving (0 = just started diving)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* TerminalVelCurve = nullptr;

	//the float curve for the terminal velocity after diving (0 = just stopped diving)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curves")
	UCurveFloat* AfterDiveTerminalVelCurve = nullptr;

	//the gravity scale to use when diving
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GravityScaleMultiplier = 8;

	//the time when the player started diving
	float StartTime = 0;

	//the time when the player stopped diving
	float StopTime = 0;

	//storage for the owner as a player character
	UPROPERTY()
	class APlayerCharacter* PlayerCharacter = nullptr;

	//constructor
	UDiveComponent();

	//override(s)
	virtual void BeginPlay() override;

	//function to start Diving
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartDive();

	//function to stop Diving
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopDive();

	//function to get whether or not the player is currently Diving
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsDiving() const;

	//function to get the terminal velocity multipliers used when falling
	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetTerminalVelMultipliers(const FVector& Vector, const FVector& Gravity, float DeltaTime);
};
