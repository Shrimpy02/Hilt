// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Math/InterpShorthand.h"
#include "GameFramework/SpringArmComponent.h"
#include "CameraArmComponent.generated.h"

//struct for the camera zoom interpolations
USTRUCT(BlueprintType)
struct FCameraZoomStruct
{
	GENERATED_BODY()

	FCameraZoomStruct(){} //default constructor

	FCameraZoomStruct(const float InSpeedThreshold, const TEnumAsByte<EInterpToTargetType> InInterpolationType, const float InTargetArmLength, const float InInterpSpeed)
	{
		SpeedThreshold = InSpeedThreshold;
		InterpType = InInterpolationType;
		TargetArmLength = InTargetArmLength;
		InterpSpeed = InInterpSpeed;
	}

	//the speed the owner needs to be moving at for this lerp to be used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedThreshold = 0;

	//the interpolation type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EInterpToTargetType> InterpType = InterpTo;

	//the target value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetArmLength = 0;

	//the interpolation speed (sometimes also used as the interpolation control)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = 0.0f))
	float InterpSpeed = 5;

};

/**
 * 
 */
UCLASS()
class UCameraArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

public:

	//constructor
	explicit UCameraArmComponent(FObjectInitializer const& ObjectInitializer);

	//whether or not to use camera zoom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings|Zoom")
	bool bUseCameraZoom = true;

	//array of camera interp structs to use for the camera zoom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings|Zoom", meta=(editcondition = "bUseCameraZoom", editconditionHides, ShowOnlyInnerProperties))
	TArray<FCameraZoomStruct> CamZoomInterps = {
		FCameraZoomStruct(0, InterpTo, 250, 5),
		FCameraZoomStruct(350, InterpTo, 500, 1),
		FCameraZoomStruct(700, InterpTo, 750, 1),
	};

	//the speed at which to update the camera zoom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings|Zoom", meta=(editcondition = "bUseCameraZoom", editconditionHides))
	float CameraZoomUpdateSpeed = 0.025f;

	//whether or not to ignore the z velocity of our owner when calculating the camera zoom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings|Zoom")
	bool bIgnoreZVelocity = true;

	//the multiplier to apply to z velocity when calculating the camera zoom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings|Zoom", meta = (editcondition = "bIgnoreZVelocity", editconditionHides))
	float ZVelocityMultiplier = 1;

	//whether or not to smooth camera movement when character crouches (character class must call OnCrouch when crouching)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings|Crouch")
	bool bSmoothCrouch = true;

	//the speed at which the camera will lerp to the new target offset when the character crouches
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings|Crouch", meta=(ClampMin = 0.0f, editcondition = "bSmoothCrouch", editconditionHides))
	float CrouchLerpSpeed = 5;

	//the time between each crouch lerping (in seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings|Crouch", meta=(ClampMin = 0.0f, editcondition = "bSmoothCrouch", editconditionHides))
	float CrouchLerpTime = 0.025f;

	//the target offset vector to use for the x value of the input vector
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings|Movement")
	FVector RightVec = FVector(0, 1, 0);

	//the target offset vector to use for the x value of the input vector
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings|Movement")
	FVector ForwardVec = FVector(1, 0, 0);

	//the offset amount to use for the offset vectors
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings|Movement")
	float MoveOffsetAmount = 50;

	//the movement offset interpolation speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings|Movement")
	float MoveOffsetInterpSpeed = 1;

	//whether or not to use the movement offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings|Movement")
	bool bUseMoveOffset = true;

	//the value to clamp the z value of the target offset to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraSettings|Movement")
	float TargetOffsetZClamp = 0;

	//storage for the player character as a player character
	UPROPERTY()
	class APlayerCharacter* PlayerCharacter = nullptr;

	//original target offset
	UPROPERTY()
	FVector OriginalTargetOffset;

	//the current index in the CamZoomInterps array to use
	float CurrentInterpIndex = 0;

	//the default target offset
	FVector DefaultTargetOffset;

	//timer handle for the zoom interpolation
	FTimerHandle ZoomInterpTimerHandle;

	//timer handle for the crouch lerping
	FTimerHandle CrouchLerpTimerHandle;

	//overrides
	virtual void BeginPlay() override;
	virtual void UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime) override;

	//function to clamp the z value of the target offset
	FVector ClampTargetOffsetZ(FVector InVector) const;

	//function that interpolates the camera zoom
	void InterpCameraZoom();

	//function for when the character crouches
	void OnStartCrouch(float ScaledHalfHeightAdjust);

	//function for when the character stops crouching
	void OnEndCrouch(float ScaledHalfHeightAdjust);

	//function that lerps the camera arm's target offset Z value
	void LerpCameraOffset();
};
