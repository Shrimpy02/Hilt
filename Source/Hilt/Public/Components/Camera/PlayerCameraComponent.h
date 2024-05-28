// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Core/Math/InterpShorthand.h"
#include "PlayerCameraComponent.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FCameraStateStruct
{
	GENERATED_BODY()

	//constructors
	explicit FCameraStateStruct(const float InFov = 90.0f, const FPostProcessSettings InPostProcessSettings = FPostProcessSettings())
	{
		PostProcessSettings = InPostProcessSettings;
		FieldOfView = InFov;
	}

	//the post process settings to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPostProcessSettings PostProcessSettings = FPostProcessSettings();

	//the Fov to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FieldOfView = 90.0f;

	//the interpolation method to use when lerping the fov
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EInterpToTargetType> FovInterpType = InterpTo;

	//the speed at which to lerp the fov
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FovInterpSpeed = 10.0f;
};

UCLASS()
class UPlayerCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

	public:

	//constructor
	UPlayerCameraComponent();

	//the current camera state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Camera")
	FCameraStateStruct CurrentCameraState;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
};