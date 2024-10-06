#include "Components/GrapplingHook/BaseGrapplingComponent.h"

UBaseGrapplingComponent::UBaseGrapplingComponent()
{
	bAutoActivate = true;
}

FVector UBaseGrapplingComponent::DoInterpGrapple(const float DeltaTime, const FVector CurrentVel, const FVector TargetPos, const float Speed, const float Acceleration)
{
	//get the direction to the target position
	const FVector Direction = (TargetPos - GetOwner()->GetActorLocation()).GetSafeNormal();

	//get the velocity to the target position
	const FVector Vel = Direction * Speed;

	//interpolate the velocity
	return FMath::VInterpTo(CurrentVel, Vel, DeltaTime, Acceleration);
}

FVector UBaseGrapplingComponent::DoPullGrapple(const float DeltaTime, const FVector CurrentVel, const FVector TargetPos, const float Speed)
{
	//get the direction to the target position
	const FVector Direction = (TargetPos - GetOwner()->GetActorLocation()).GetSafeNormal();

	////get the quaternion to transform to the grapple space
	//const FQuat WorldToGrappleTransform = FQuat::FindBetweenNormals(Direction,  -CurrentVel.GetSafeNormal());

	////get the velocity in the grapple space
	//FVector GrappleSpaceVel = WorldToGrappleTransform.RotateVector(CurrentVel);

	//////set the velocity
	////GrappleSpaceVel.Z = 0;

	////get the velocity back in world space
	//FVector NewVelDir = WorldToGrappleTransform.Inverse().RotateVector(GrappleSpaceVel).GetSafeNormal();

	//storage for the velocity that will be applied from the grapple
	return CurrentVel + Direction * DeltaTime * Speed * GetPullGrappleModifiers(Direction);
}

float UBaseGrapplingComponent::GetPullGrappleModifiers(FVector Direction) const
{
	//return the default value
	return 1;
}
