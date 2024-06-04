#include "Components/GrapplingHook/RopeComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Core/HiltTags.h"
#include "NPC/Components/GrappleableComponent.h"

URopeComponent::URopeComponent()
{
	//initialize the rope points array
	RopePoints.Init(FVector(), 2);

	//add the no grapple tag
	ComponentTags.Add(HiltTags::NoGrappleTag);

	//set the tick group and behavior
	TickGroup = TG_PostUpdateWork;

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

		//update first and last points or Hitboxes
		SetAttachedRopePointPositions();

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
			GetWorld()->SweepSingleByChannel(Surrounding, RopePoints[Index - 1], RopePoints[Index + 1], FQuat(), ECC_Visibility, FCollisionShape::MakeSphere(RopeRadius), CollisionParams);

			//check if the sweep returned a blocking hit or started inside an object
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
		}

		//hit result to check for new rope points
		FHitResult Next;

		//sweep from the current rope point to the next rope point
		GetWorld()->SweepSingleByChannel(Next, RopePoints[Index], RopePoints[Index + 1], FQuat(), ECC_Visibility, FCollisionShape::MakeSphere(RopeRadius), CollisionParams);

		//check for hits
		if (Next.IsValidBlockingHit())
		{
			////print the name of the component we hit
			//GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, Next.GetComponent()->GetName());

			//if we hit something, add a new rope point at the hit location if we're not too close to the last rope point
			if (FVector::Dist(RopePoints[Index], Next.Location) > MinCollisionPointSpacing && FVector::Dist(RopePoints[Index + 1], Next.Location) > MinCollisionPointSpacing && !Next.bStartPenetrating && Next.Location != Next.TraceEnd && Next.Location != Next.TraceStart)
			{
				//insert the new rope point at the hit location
				RopePoints.Insert(Next.Location + Next.ImpactNormal, Index + 1);
			}
		}
	}
}

void URopeComponent::SetAttachedRopePointPositions()
{
	//set the start and end rope points
	RopePoints[0] = GetComponentLocation();
	RopePoints[RopePoints.Num() - 1] = GetRopeEnd();
}

void URopeComponent::SpawnNiagaraSystem(int Index)
{
	//create a new Niagara component
	UNiagaraComponent* NewNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraSystem, RopePoints[Index]);

	//set the end location of the Niagara component
	NewNiagaraComponent->SetVectorParameter(RibbonEndParameterName, RopePoints[Index + 1]);

	//set tick group and behavior
	NewNiagaraComponent->SetTickGroup(TickGroup);
	NewNiagaraComponent->SetTickBehavior(TickBehavior);

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
			DrawDebugLine(GetWorld(), RopePoints[Index], RopePoints[Index + 1], FColor::Red, false, 0.f, 0, 5.f);
		}
	}
	//check if we don't have a valid Niagara system to render
	else if (NiagaraSystem->IsValidLowLevelFast())
	{
		//check if we have a valid Niagara system to render
		if (NiagaraSystem->IsValidLowLevelFast())
		{
			//iterate through all the rope points except the last one
			for (int Index = 0; Index < RopePoints.Num() - 1; ++Index)
			{
				//check if we have a valid Niagara component to use or if we need to create a new one
				if (NiagaraComponents.IsValidIndex(Index) && NiagaraComponents[Index]->IsValidLowLevelFast())
				{
					//set the start location of the Niagara component
					NiagaraComponents[Index]->SetWorldLocation(RopePoints[Index]);

					//set the end location of the Niagara component
					NiagaraComponents[Index]->SetVectorParameter(RibbonEndParameterName, RopePoints[Index + 1]);

					//check if we should use rope radius
					if (UseRopeRadiusAsRibbonWidth)
					{
						//set the ribbon width to the rope radius
						NiagaraComponents[Index]->SetFloatParameter(RibbonWidthParameterName, RopeRadius /* * 2*/);
					}
					else
					{
						//set the ribbon width to the ribbon width
						NiagaraComponents[Index]->SetFloatParameter(RibbonWidthParameterName, RibbonWidth);
					}
				}
				else
				{
					//create a new Niagara component
					SpawnNiagaraSystem(Index);
				}
			}
		}
		else
		{
			//draw debug message
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("RopeRenderer has no valid niagara system"));
		}
	}
	else
	{
		//draw debug message
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Draw debug is false, and we don't have a valid niagara system"));
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

	//clear the rope points array
	RopePoints.Empty();
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void URopeComponent::ActivateRope(AActor* OtherActor, const FHitResult& HitResult)
{
	//set the start hit
	StartHit = HitResult;

	//set the grappleable component
	this->GrappleableComponent = OtherActor->FindComponentByClass<UGrappleableComponent>();

	//set the active state to true
	bIsRopeActive = true;

	//initialize the rope points array
	RopePoints.Init(FVector(), 2);

	//set the attached rope point positions
	SetAttachedRopePointPositions();
}

float URopeComponent::GetRopeLength() const
{
	//initialize the rope length
	float Length = 0.f;

	//iterate through all the rope points except the last one
	for (int Index = 0; Index < RopePoints.Num() - 1; ++Index)
	{
		//add the distance between the current rope point and the next rope point to the rope length
		Length += FVector::Dist(RopePoints[Index], RopePoints[Index + 1]);
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

	//return the location of the start hit
	return StartHit.ImpactPoint;
}

FVector URopeComponent::GetSecondRopePoint() const
{
	return RopePoints[1];
}
