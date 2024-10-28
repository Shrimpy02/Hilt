#include "Components/Camera/CameraArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/PlayerCharacter.h"

//maybe add a setting

UCameraArmComponent::UCameraArmComponent(const FObjectInitializer& ObjectInitializer)
{
	//set the default values
	//CameraLagMaxDistance = 800;
	//ProbeSize = 120;
}

void UCameraArmComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	////set default target offset
	//DefaultTargetOffset = TargetOffset;

	////check if we smooth cameramovement when crouching
	//if (bSmoothCrouch)
	//{
	//	//set the timer handle
	//	GetWorld()->GetTimerManager().SetTimer(CrouchLerpTimerHandle, this, &UCameraArmComponent::LerpCameraOffset, CrouchLerpTime, true);
	//}

	////check if we use camera zoom
	//if (bUseCameraZoom)
	//{
	//	//check if camera lerps is empty
	//	if (CamZoomInterps.IsEmpty())
	//	{
	//		//add a default value to the map
	//		CamZoomInterps.Add(FCameraZoomStruct());
	//	}

	//	//set the timer handle
	//	GetWorld()->GetTimerManager().SetTimer(ZoomInterpTimerHandle, this, &UCameraArmComponent::InterpCameraZoom, CameraZoomUpdateSpeed, true);
	//}

	////set the player character
	//PlayerCharacter = Cast<APlayerCharacter>(GetOwner());

	////set the original target offset
	//OriginalTargetOffset = TargetOffset;
}

void UCameraArmComponent::UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime)
{
	////check if the player character is valid
	//if (!PlayerCharacter->IsValidLowLevelFast())
	//{
	//	//default to the parent implementation
	//	Super::UpdateDesiredArmLocation(bDoTrace, bDoLocationLag, bDoRotationLag, DeltaTime);

	//	//prevent further execution
	//	return;
	//}

	////check if we're using the movement offset
	//if (bUseMoveOffset)
	//{
	//	//get the input vector of the player character
	//	const FVector2D InputVector = PlayerCharacter->CurrentMoveDirection;

	//	//set the target offset
	//	TargetOffset = FMath::VInterpTo(TargetOffset, OriginalTargetOffset + (RightVec * InputVector.X + ForwardVec * InputVector.Y) * -MoveOffsetAmount, DeltaTime, MoveOffsetInterpSpeed);
	//}

	//defualt to the parent implementation
	Super::UpdateDesiredArmLocation(bDoTrace, bDoLocationLag, bDoRotationLag, DeltaTime);
}

FVector UCameraArmComponent::ClampTargetOffsetZ(FVector InVector) const
{
	//check if the z value is above the target offset z clamp
	if (InVector.Z > TargetOffsetZClamp)
	{
		//set the z value to the target offset z clamp
		InVector.Z = TargetOffsetZClamp;
	}
	else if (InVector.Z < -TargetOffsetZClamp)
	{
		//set the z value to the negative target offset z clamp
		InVector.Z = -TargetOffsetZClamp;
	}

	//return the clamped vector
	return InVector;

}

void UCameraArmComponent::InterpCameraZoom()
{
	//the speed of movement
	float Speed;

	//check if we should ignore the z axis
	if (bIgnoreZVelocity)
	{
		//set the speed in 2d
		Speed = GetOwner()->GetVelocity().Size2D();
	}
	else
	{
		//get the difference betwen the 2d speed and the 3d speed
		const float SpeedDifference = GetOwner()->GetVelocity().Size2D() - GetOwner()->GetVelocity().Length();

		//set the speed to the 2d speed + the speed difference multiplied by the z velocity multiplier
		Speed = GetOwner()->GetVelocity().Size2D() + SpeedDifference * ZVelocityMultiplier;
	}

	//check if current interp index is above 0
	if (CurrentInterpIndex > 0)
	{
		//check if the speed is below the current interp's speed threshold
		if (Speed < CamZoomInterps[CurrentInterpIndex].SpeedThreshold)
		{
			//decrease the current interp index
			CurrentInterpIndex--;
		}
	}

	//check if current interp index is below the max index
	if (CurrentInterpIndex < CamZoomInterps.Num() - 1)
	{
		//check if the speed is above the current interp's speed threshold
		if (Speed > CamZoomInterps[CurrentInterpIndex].SpeedThreshold)
		{
			//increase the current interp index
			CurrentInterpIndex++;
		}
	}
	//get the world delta time
	const float WorldDeltaTime = GetWorld()->GetDeltaSeconds();

	//get the new target arm length
	const float NewTargetArmLength = InterpToTarget<float>(CamZoomInterps[CurrentInterpIndex].InterpType, TargetArmLength, CamZoomInterps[CurrentInterpIndex].TargetArmLength, WorldDeltaTime, CamZoomInterps[CurrentInterpIndex].InterpSpeed);

	//apply the new target arm length
	TargetArmLength = NewTargetArmLength;
}

void UCameraArmComponent::OnStartCrouch(const float ScaledHalfHeightAdjust)
{
	//set the target offset's z value
	TargetOffset.Z = DefaultTargetOffset.Z + ScaledHalfHeightAdjust;
}

void UCameraArmComponent::OnEndCrouch(const float ScaledHalfHeightAdjust)
{
	//set the target offset's z value
	TargetOffset.Z = DefaultTargetOffset.Z - ScaledHalfHeightAdjust;
}

void UCameraArmComponent::LerpCameraOffset()
{
	//get the world delta time
	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	//lerp the target offset
	TargetOffset.Z = UKismetMathLibrary::FInterpTo(TargetOffset.Z, DefaultTargetOffset.Z, DeltaTime, CrouchLerpSpeed);
}
