#include "NPC/Components/GrappleableComponent.h"

UGrappleableComponent::UGrappleableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGrappleableComponent::OnStartGrapple(const FHitResult& HitResult)
{
	//get the hit location as relative to this actor
	const FVector HitLocation = GetComponentTransform().InverseTransformPosition(HitResult.Location);

	//set the component's relative location to the relative hit location
	SetRelativeLocation(HitLocation);

	//broadcast the event
	OnStartGrappleEvent.Broadcast(HitResult);
}

void UGrappleableComponent::OnCollisionGrapple(AActor* OtherActor, const FHitResult& HitResult)
{
	//broadcast the event
	OnCollisionGrappleEvent.Broadcast(OtherActor, HitResult);
}

void UGrappleableComponent::OnStopGrapple()
{
	//reset the component's relative location
	SetRelativeLocation(FVector::ZeroVector);

	//broadcast the event
	OnStopGrappleEvent.Broadcast();
}
