#include "Components/GrapplingHook/RopeComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Core/HiltTags.h"
#include "NPC/Components/GrappleableComponent.h"

FRopePoint::FRopePoint()
{
}

FVector FRopePoint::GetWL() const
{
	return AttachedActor->GetTransform().TransformPosition(RelativeLocation);

}

FRopePoint::FRopePoint(const FHitResult& HitResult)
{
	//set the attached actor
	AttachedActor = HitResult.GetActor();

	//set the relative location
	RelativeLocation = AttachedActor->GetTransform().InverseTransformPosition(HitResult.ImpactPoint);
}

FRopePoint::FRopePoint(AActor* OtherActor, const FVector& Location)
{
	//set the attached actor
	AttachedActor = OtherActor;

	//set the relative location
	RelativeLocation = OtherActor->GetTransform().InverseTransformPosition(Location);
}

URopeComponent::URopeComponent()
{
	//add the no grapple tag
	ComponentTags.Add(HiltTags::NoGrappleTag);

	////set the tick group and behavior
	//TickGroup = TG_PostUpdateWork;

	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;
	UActorComponent::SetComponentTickEnabled(true);
}

void URopeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//call the parent implementation
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//check if we're grappling
	if (bIsRopeActive)
	{
		//update the rope points
		CheckCollisionPoints();

		//render the rope
		RenderRope();
	}
}

void URopeComponent::DestroyComponent(const bool bPromoteChildren)
{
	//destroy all the niagara components
	for (UNiagaraComponent* NiagaraComponent : NiagaraComponents)
	{
		NiagaraComponent->DestroyComponent();
	}

	//call the parent implementation
	Super::DestroyComponent(bPromoteChildren);
}

void URopeComponent::SetNiagaraSystem(UNiagaraSystem* NewSystem)
{
	//set the new niagara system
	NiagaraSystem = NewSystem;

	//iterate through all the niagara components
	for (UNiagaraComponent* NiagaraComponent : NiagaraComponents)
	{
		//set the new niagara system
		NiagaraComponent->SetAsset(NiagaraSystem);
	}
}

void URopeComponent::CheckCollisionPoints()
{
	//setup collision parameters for traces and sweeps
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());

	//iterate through all the rope points
	for (int Index = 0; Index < RopePoints.Num() - 1; Index++)
	{
		//check if we're not at the first rope point
		if (Index != 0)
		{
			//sweep from the previous rope point to the next rope point
			FHitResult Surrounding;
			//GetWorld()->SweepSingleByChannel(Surrounding, RopePoints[Index - 1].GetWL(), RopePoints[Index + 1].GetWL(), FQuat(), ECC_Visibility, FCollisionShape::MakeSphere(RopeRadius), CollisionParams);
			GetWorld()->LineTraceSingleByChannel(Surrounding, RopePoints[Index - 1].GetWL(), RopePoints[Index + 1].GetWL(), ECC_Visibility, CollisionParams);
			//DrawDebugLine(GetWorld(), RopePoints[Index - 1].GetWL(), RopePoints[Index + 1].GetWL(), FColor::Blue, false, 0.f, 0, 5.f);

			//check if the sweep didn't return a blocking hit and didn't started inside an object
			if (!Surrounding.bBlockingHit && !Surrounding.bStartPenetrating)
			{
				//remove the rope point from the array
				RopePoints.RemoveAt(Index);

				//check if we need to remove the niagara component for this rope point
				if (NiagaraComponents.IsValidIndex(Index) && NiagaraComponents[Index]->IsValidLowLevelFast())
				{
					//destroy the niagara component
					NiagaraComponents[Index]->DestroyComponent();
					
					//remove the niagara component from the array
					NiagaraComponents.RemoveAt(Index);
				}

				//decrement i so we don't skip the next rope point
				Index--;

				//continue to the next rope point
				continue;
			}

			////print whether the hit started penetrating and whether the hit was a blocking hit
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Start Penetrating: %d, Blocking Hit: %d"), Surrounding.bStartPenetrating, Surrounding.bBlockingHit));
		}
		//hit result to check for new rope points
		FHitResult Next;

		//sweep from the current rope point to the next rope point
		//GetWorld()->SweepSingleByChannel(Next, RopePoints[Index].GetWL(), RopePoints[Index + 1].GetWL(), FQuat(), ECC_Visibility, FCollisionShape::MakeSphere(RopeRadius), CollisionParams);
		GetWorld()->LineTraceSingleByChannel(Next, RopePoints[Index].GetWL(), RopePoints[Index + 1].GetWL(), ECC_Visibility, CollisionParams);


		//check for hits
		if (Next.IsValidBlockingHit())
		{
			//if we hit something, add a new rope point at the hit location if we're not too close to the last rope point
			if (FVector::Dist(RopePoints[Index].GetWL(), Next.Location) > MinCollisionPointSpacing && FVector::Dist(RopePoints[Index + 1].GetWL(), Next.Location) > MinCollisionPointSpacing)
			{
				////insert the new rope point at the hit location
				//RopePoints.Insert(Next.Location + Next.ImpactNormal * 10, Index + 1);

				//insert the new rope point at the correct tarray index
				RopePoints.Insert(FRopePoint(Next), Index + 1);
			}

			//DrawDebugLine(GetWorld(), RopePoints[Index].GetWL(), RopePoints[Index + 1].GetWL(), FColor::Yellow, false, 0.f, 0, 5.f);
		}
	}
}

void URopeComponent::SpawnNiagaraSystem(int Index)
{
	//create a new Niagara component
	UNiagaraComponent* NewNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraSystem, RopePoints[Index].GetWL());

	//set the end location of the Niagara component
	NewNiagaraComponent->SetVectorParameter(RibbonEndParameterName, RopePoints[Index + 1].GetWL());

	//set tick group and behavior
	NewNiagaraComponent->SetTickGroup(TG_LastDemotable);
	NewNiagaraComponent->SetTickBehavior(ENiagaraTickBehavior::UseComponentTickGroup);

	//add the no grapple tag to the Niagara component
	NewNiagaraComponent->ComponentTags.Add(HiltTags::NoGrappleTag);

	//add the new Niagara component to the array
	NiagaraComponents.Add(NewNiagaraComponent);
}

void URopeComponent::RenderRope()
{
	//chceck if we should use debug drawing
	if (bUseDebugDrawing)
	{
		//iterate through all the rope points except the last one
		for (int Index = 0; Index < RopePoints.Num() - 1; ++Index)
		{
			//draw a debug line between the current rope point and the next rope point
			DrawDebugLine(GetWorld(), RopePoints[Index].GetWL(), RopePoints[Index + 1].GetWL(), FColor::Red, false, 0.f, 0, 5.f);
		}

		//return to prevent further execution
		return;
	}

	//check if we don't have a valid Niagara system to render
	if (!NiagaraSystem->IsValidLowLevelFast())
	{
		//draw debug message
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Draw debug is false, and we don't have a valid niagara system"));

		//return to prevent further execution
		return;
	}

	//iterate through all the rope points except the last one
	for (int Index = 0; Index < RopePoints.Num() - 1; ++Index)
	{
		//check if we have a valid Niagara component to use or if we need to create a new one
		if (NiagaraComponents.IsValidIndex(Index) && NiagaraComponents[Index]->IsValidLowLevelFast())
		{
			//set the start location of the Niagara component
			NiagaraComponents[Index]->SetWorldLocation(RopePoints[Index].GetWL());

			//set the end location of the Niagara component
			NiagaraComponents[Index]->SetVectorParameter(RibbonEndParameterName, RopePoints[Index + 1].GetWL());
		}
		else
		{
			//create a new Niagara component
			SpawnNiagaraSystem(Index);
		}
	}
}

void URopeComponent::DeactivateRope()
{
	//set the active state to false
	bIsRopeActive = false;

	//iterate through all the niagara components
	for (UNiagaraComponent* NiagaraComponent : NiagaraComponents)
	{
		//destroy the niagara component
		NiagaraComponent->DestroyComponent();
	}

	//clear the niagara components array
	NiagaraComponents.Empty();
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void URopeComponent::ActivateRope(AActor* OtherActor, const FHitResult& HitResult)
{
	//set the grappleable component
	this->GrappleableComponent = OtherActor->FindComponentByClass<UGrappleableComponent>();

	//set the active state to true
	bIsRopeActive = true;

	//set the rope points
	RopePoints = { FRopePoint(GetOwner(), GetComponentLocation()), FRopePoint(HitResult) };
}

float URopeComponent::GetRopeLength() const
{
	//initialize the rope length
	float Length = 0.f;

	//iterate through all the rope points except the last one
	for (int Index = 0; Index < RopePoints.Num() - 1; ++Index)
	{
		//add the distance between the current rope point and the next rope point to the rope length
		Length += FVector::Dist(RopePoints[Index].GetWL(), RopePoints[Index + 1].GetWL());
	}

	//return the rope length
	return Length;
}

FVector URopeComponent::GetRopeEnd() const
{
	//check if we have a valid grappleable component
	if (GrappleableComponent)
	{
		//return the location of the grappleable component
		return GrappleableComponent->GetComponentLocation();
	}

	//return the current world location of the relative location
	return RopePoints.Last().GetWL();
}

FVector URopeComponent::GetSecondRopePoint() const
{
	return RopePoints[1].GetWL();
}
