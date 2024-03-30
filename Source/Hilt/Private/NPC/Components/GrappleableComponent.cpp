#include "NPC/Components/GrappleableComponent.h"

UGrappleableComponent::UGrappleableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGrappleableComponent::OnStartGrapple(AActor* OtherActor, const FHitResult& HitResult)
{
	//get the hit location as relative to this actor
	const FVector HitLocation = GetComponentTransform().InverseTransformPosition(HitResult.Location);

	//set the component's relative location to the relative hit location
	SetRelativeLocation(HitLocation);
}

void UGrappleableComponent::OnStopGrapple()
{
	//reset the component's relative location
	SetRelativeLocation(FVector::ZeroVector);
}

void UGrappleableComponent::WhileGrappled(float DeltaTime)
{
	//empty default implementation
}
