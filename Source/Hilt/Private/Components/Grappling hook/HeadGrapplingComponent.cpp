#include "Components/GrapplingHook/HeadGrapplingComponent.h"

#include "Components/GrapplingHook/RopeComponent.h"
#include "NPC/Components/GrappleableComponent.h"

UHeadGrapplingComponent::UHeadGrapplingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UHeadGrapplingComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//set the rope component
	RopeComponent = GetOwner()->FindComponentByClass<URopeComponent>();

	//bind the start and stop grapple events
	OnStartGrapple.AddDynamic(RopeComponent, &URopeComponent::ActivateRope);
	OnStopGrapple.AddDynamic(RopeComponent, &URopeComponent::DeactivateRope);
}

void UHeadGrapplingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//call the parent implementation
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//check if we're grappling
	if (IsGrappling())
	{
		//check if we should autostop the grapple
		if (ShouldAutostopGrapple())
		{
			//stop grappling
			StopGrapple();
		}
	}
}

bool UHeadGrapplingComponent::IsGrappling() const
{
	//return bIsGrappling
	return false;
}

FVector UHeadGrapplingComponent::GetEndLocation() const
{
	//check if we have a valid rope component
	if (RopeComponent->IsValidLowLevelFast())
	{
		//return the rope end
		return RopeComponent->GetRopeEnd();
	}

	//return the zero vector
	return FVector::Zero();
}

FVector UHeadGrapplingComponent::GetGrappleDirection() const
{
	return (GetEndLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
}

FCollisionQueryParams UHeadGrapplingComponent::GetCollisionParams() const
{
	//create the collision query params
	FCollisionQueryParams CollisionParams;

	//set the ignore actors
	CollisionParams.AddIgnoredActor(GetOwner());

	//return the collision params
	return CollisionParams;

}

void UHeadGrapplingComponent::StartGrapple(const FHitResult& HitResult)
{
	//check if the hit result is not blocking
	if (!HitResult.bBlockingHit)
	{
		//an error occurred, print an error message
		UE_LOG(LogTemp, Error, TEXT("The hit result is not blocking"));

		//return early
		return;
	}

	//check if the hit actor is not valid
	if (!HitResult.GetActor())
	{
		//an error occurred, print an error message
		UE_LOG(LogTemp, Error, TEXT("The hit actor is not valid"));

		//return early
		return;
	}

	//set the grapple start time
	GrappleStartTime = GetWorld()->GetTimeSeconds();

	//set the grapple target
	GrappleTarget = HitResult.GetActor();

	//check if the delegate is not already bound
	if (!GrappleTarget->OnDestroyed.IsAlreadyBound(this, &UHeadGrapplingComponent::OnGrappleTargetDestroyed))
	{
		//bind the destroyed event to the grapple target (so we can stop grappling if the target is destroyed)
		GrappleTarget->OnDestroyed.AddDynamic(this, &UHeadGrapplingComponent::OnGrappleTargetDestroyed);
	}

	//check if the other actor has a grappleable component
	if (GrappleableComponent = GrappleTarget->GetComponentByClass<UGrappleableComponent>(); GrappleableComponent->IsValidLowLevelFast())
	{
		//set bisgrappled to true on the grappleable component
		GrappleableComponent->bIsGrappled = true;

		//bind the events to this component
		OnStartGrapple.AddDynamic(GrappleableComponent, &UGrappleableComponent::OnStartGrapple);
		OnStopGrapple.AddDynamic(GrappleableComponent, &UGrappleableComponent::OnStopGrapple);
	}

	//call the OnStartGrapple event
	OnStartGrapple.Broadcast(HitResult);
}

void UHeadGrapplingComponent::StopGrapple(bool CallBlueprintEvent)
{
	//check if the grapple target is valid
	if (GrappleTarget->IsValidLowLevelFast())
	{
		//check if the delegate is already bound
		if (GrappleTarget->OnDestroyed.IsAlreadyBound(this, &UHeadGrapplingComponent::OnGrappleTargetDestroyed))
		{
			//unbind the destroyed event from the grapple target (so we don't risk the next grapple to an unrelated actor getting stopped due to this actor being destroyed)
			GrappleTarget->OnDestroyed.RemoveDynamic(this, &UHeadGrapplingComponent::OnGrappleTargetDestroyed);	
		}
	}

	//check if we should call the blueprint event
	if (CallBlueprintEvent)
	{
		//call the OnStopGrapple event
		OnStopGrapple.Broadcast();
	}

	//check if we have a grappleable component
	if (GrappleableComponent->IsValidLowLevelFast())
	{
		//set bisgrappled to false
		GrappleableComponent->bIsGrappled = false;

		//unbind the events from this component
		OnStartGrapple.RemoveDynamic(GrappleableComponent, &UGrappleableComponent::OnStartGrapple);
		OnStopGrapple.RemoveDynamic(GrappleableComponent, &UGrappleableComponent::OnStopGrapple);
	}
}

bool UHeadGrapplingComponent::ShouldAutostopGrapple() const
{
	//check if we're too close to the rope end
	if (FVector::Dist(GetOwner()->GetActorLocation(), RopeComponent->GetRopeEnd()) < GrappleStopDistance)
	{
		return true;
	}

	//check if the grapple target is valid
	if (!GrappleTarget->GetLevel()->bIsVisible)
	{
		return true;
	}

	return false;
}

void UHeadGrapplingComponent::OnGrappleTargetDestroyed(AActor* DestroyedActor)
{
	//stop grappling
	StopGrapple();
}
