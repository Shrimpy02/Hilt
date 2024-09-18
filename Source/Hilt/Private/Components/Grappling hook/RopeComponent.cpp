#include "Components/GrapplingHook/RopeComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
//#include "math.h"
#include "Core/HiltTags.h"
#include "NPC/Components/GrappleableComponent.h"
#include "Player/PlayerCharacter.h"

FVerletConstraint::FVerletConstraint()
{
}

FVerletConstraint::FVerletConstraint(FRopePoint* InStartPoint, FRopePoint* InEndPoint, const float InCompensation1, const float InCompensation2, const float InDistance)
{
	//set the start point of the constraint
	StartPoint = InStartPoint;

	//set the end point of the constraint
	EndPoint = InEndPoint;

	//set the compensation to apply to the first point of the constraint
	Compensation1 = InCompensation1;

	//set the compensation to apply to the second point of the constraint
	Compensation2 = InCompensation2;

	//set the distance between the two points of the constraint
	Distance = InDistance;

}

FVector FVerletConstraint::GetStartPoint() const
{
	return StartPoint->GetWL();
}

FVector FVerletConstraint::GetEndPoint() const
{
	return EndPoint->GetWL();
}

float FVerletConstraint::GetDistance() const
{
	return Distance;
}

void FVerletConstraint::SetStartPoint(const FVector& NewStartPoint) const
{
	StartPoint->SetWL(NewStartPoint);
}

void FVerletConstraint::SetEndPoint(const FVector& NewEndPoint) const
{
	EndPoint->SetWL(NewEndPoint);
}

void FVerletConstraint::SetDistance(const float NewDistance)
{
	Distance = NewDistance;
}

FRopePoint::FRopePoint()
{
}

FRopePoint::FRopePoint(FVector InLocation, const bool bInUseWorldSpace) : Location(InLocation), bUseWorldSpace(bInUseWorldSpace)
{
}

FVector FRopePoint::GetWL() const
{
	//check if we're using world space for this rope point
	if (bUseWorldSpace)
	{
		//return the world location of the component
		return Location;
	}

	//check if we have a attached actor
	if (AttachedActor)
	{
		//get the grapplable component of the attached actor
		if (const UGrappleableComponent* GrappleableComponent = AttachedActor->FindComponentByClass<UGrappleableComponent>())
		{
			//return the location of the grappleable component
			return GrappleableComponent->GetComponentLocation();
		}
	}

	//check if we're using a component for this rope point
	if (Component->IsValidLowLevelFast())
	{
		//check if the component has a socket named "GrapplingHookSocket" and that the component is visible
		if (Component->DoesSocketExist("GrapplingHookSocket") && Component->IsVisible())
		{
			//default to the relative location transformed by the attached actor's transform
			return Component->GetSocketLocation("GrapplingHookSocket");
		}

		//default to components location
		return Component->GetComponentLocation();
	}

	//check if we're using an actor for this rope point
	if (AttachedActor)
	{
		//default to the relative location transformed by the attached actor's transform
		return AttachedActor->GetTransform().TransformPosition(Location);
	}

	//default to the zero vector to prevent crashes and still have some indication that something went wrong
	return FVector::ZeroVector;
}

void FRopePoint::SetWL(const FVector& NewLocation)
{
	//todo add force so that the rope can pull an attached actor

	//check if we're using world location
	if (bUseWorldSpace)
	{
		//set the relative location
		Location = NewLocation;
	}
}

FRopePoint::FRopePoint(const FHitResult& HitResult)
{
	//set the attached actor
	AttachedActor = HitResult.GetActor();

	//set the relative location
	Location = AttachedActor->GetTransform().InverseTransformPosition(HitResult.ImpactPoint);

	//set the collision point flag
	bIsCollisionPoint = true;
}

FRopePoint::FRopePoint(AActor* InOtherActor, const FVector& InLocation)
{
	//set the attached actor
	AttachedActor = InOtherActor;

	//set the relative location
	Location = InOtherActor->GetTransform().InverseTransformPosition(InLocation);
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

void URopeComponent::BeginPlay()
{
	//call the parent implementation
	Super::BeginPlay();

	//cast the owner to a player character
	if (APlayerCharacter* LocPlayerCharacter = Cast<APlayerCharacter>(GetOwner()))
	{
		//set the player character
		PlayerCharacter = LocPlayerCharacter;
	}
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

		if (bUseVerletIntegration)
		{
			//perform the verlet integration
			VerletIntegration(DeltaTime);
		}
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

void URopeComponent::EnforceConstraints()
{
	//do a number of iterations to enforce the constraints
	for (int i = 0; i < NumConstraintIterations; i++)
	{
		//iterate through all the constraints
		for (FVerletConstraint& Constraint : Constraints)
		{
			//get the delta between the start and end points
			const FVector Delta = Constraint.GetStartPoint() - Constraint.GetEndPoint();

			//get the delta length and check if it's greater than the constraint's distance
			if (const float DeltaLength = Delta.Size(); DeltaLength > 0)
			{
				//get the difference between the delta length and the distance
				const float Diff = (DeltaLength - Constraint.GetDistance()) / DeltaLength;

				//check if the start point compensation is not null
				if (Constraint.Compensation1 != 0)
				{
					//calculate the new position of the start point
					const FVector NewPosition = Constraint.GetStartPoint() - Delta * Diff * Constraint.Compensation1;

					//check for collisions and update the start point
					if(CheckForCollisions(Constraint.GetEndPoint(), NewPosition, *Constraint.StartPoint))
					{
						//add the collision point to the array (if it's not already in the array)
						CollisionPoints.AddUnique(Constraint.StartPoint);
					}
				}

				//check if the end point compensation is not null
				if (Constraint.Compensation2 != 0)
				{
					//calculate the new position of the end point
					const FVector NewPosition = Constraint.GetEndPoint() + Delta * Diff * Constraint.Compensation2;

					//check for collisions and update the end point
					if(CheckForCollisions(Constraint.GetStartPoint(), NewPosition, *Constraint.EndPoint))
					{
						//add the collision point to the array (if it's not already in the array)
						CollisionPoints.AddUnique(Constraint.EndPoint);
					}
				}
			}
		}
	}
}

bool URopeComponent::CheckForCollisions(const FVector& Start, const FVector& End, FRopePoint& Point) const
{
	//get the collision parameters
	const FCollisionQueryParams CollisionParams = GetCollisionParams();

	//storage for line trace hit result
	FHitResult Hit;

	//do a line trace from the old position to the new position
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, CollisionChannel, CollisionParams);

	//check if we hit something
	if (Hit.IsValidBlockingHit())
	{
		//get the normal of the hit
		const FVector Normal = Hit.ImpactNormal;

		//get the penetration depth
		const float PenetrationDepth = Hit.PenetrationDepth;

		//get the new position of the start point without correcting for the distance that should be traveled
		const FVector NewPosition = Hit.ImpactPoint + Normal * (PenetrationDepth + 1);

		//move the start point away from the hit (add 1 to the penetration depth to prevent the new position from being inside the hit object)
		Point.SetWL(NewPosition);
	}
	else
	{
		//set the new position of the start point
		Point.SetWL(End);
	}

	//return whether we hit something
	return Hit.IsValidBlockingHit();

}

bool URopeComponent::CheckForCollisions(const FVerletConstraint& Constraint, const FVector& InNewStartPos1, const FVector& InNewStartPos2) const
{
	//get the collision parameters
	const FCollisionQueryParams CollisionParams = GetCollisionParams();

	//get the first and second points of the constraint
	FRopePoint* StartPoint = Constraint.StartPoint;
	FRopePoint* EndPoint = Constraint.EndPoint;

	//check for collisions on the first point of the constraint
	const bool FirstTrace = CheckForCollisions(StartPoint->GetWL(), InNewStartPos1, *StartPoint);

	//check for collisions on the second point of the constraint
	const bool SecondTrace =CheckForCollisions(EndPoint->GetWL(), InNewStartPos2, *EndPoint);

	return FirstTrace && SecondTrace;
}

bool URopeComponent::CheckForCollisions(FRopePoint& Point, const FVector& InNewPosition, const FVector& OldPosition, const FVector& InVelocity, const FVector& InAcceleration) const
{
	//get the collision parameters
	const FCollisionQueryParams CollisionParams = GetCollisionParams();

	//storage for line/sweep trace hit result
	FHitResult Hit;

	//create a collision shape for the sweep
	FCollisionShape Shape = FCollisionShape();
	Shape.SetSphere(RopeRadius);

	//do a line trace from the old position to the new position
	GetWorld()->LineTraceSingleByChannel(Hit, InNewPosition, OldPosition, CollisionChannel, CollisionParams);
	//GetWorld()->SweepSingleByChannel(Hit, InNewPosition, OldPosition, FQuat(), CollisionChannel, Shape, CollisionParams);

	//check if we hit something
	if (Hit.IsValidBlockingHit())
	{
		//get the normal of the hit
		const FVector Normal = Hit.Normal;

		//get the penetration depth
		const float PenetrationDepth = Hit.PenetrationDepth;

		//get the new position of the start point and move the start point away from the hit (add 1 to the penetration depth to prevent the new position from being inside the hit object)
		const FVector NewPosition = Hit.ImpactPoint + Normal * (PenetrationDepth + 1);

		//update the start point
		Point.SetWL(NewPosition);
		Point.Velocity = InVelocity;
		Point.Acceleration = InAcceleration;
	}
	else
	{
		//update the start point
		Point.SetWL(InNewPosition);
		Point.Velocity = InVelocity;
		Point.Acceleration = InAcceleration;
	}

	//return whether we hit something
	return Hit.IsValidBlockingHit();

}

void URopeComponent::VerletIntegration(const float DeltaTime)
{
	//empty the collision points array
	CollisionPoints.Empty();

	//iterate through all the rope points
	for (FRopePoint& RopePoint : RopePoints)
	{
		////get the forces acting on the verlet point (gravity, tension, and damping)
		//FVector Gravity = {0, 0, GetWorld()->GetDefaultGravityZ() * VerletGravityFactor};

		////calculate the acceleration on the rope point
		//const FVector A = FVector(Gravity / RopePoint.Mass);

		//////calculate the new position of the verlet point
		////const FVector NewPosition = RopePoint.GetWL() + (RopePoint.GetWL() - RopePoint.OldLocations[RopePoint.OldLocations.Num() - 1]) * Damping + Acceleration * FMath::Square(DeltaTime);

		////get the current location of the rope point
		//const FVector X = RopePoint.GetWL();

		////get the old location of the rope point
		//const FVector X0 = RopePoint.OldLocations.Last();

		////get the delta time
		//const float DT = DeltaTime;

		////get the old timestep of the rope point
		//const float DT0 = RopePoint.OldTimesteps.Last();

		////calculate the new position of the verlet point (taken from https://stackoverflow.com/questions/32709599/the-time-corrected-verlet-numerical-integration-formula)
		//FVector NewPosition = X + (X - X0) * DT / DT0 + A * DT * (DT + DT0) / 2;

		//code for velocity-verlet integration

		//calculate the new position of the verlet point
		FVector NewPosition = RopePoint.GetWL() + RopePoint.Velocity * DeltaTime + RopePoint.Acceleration * FMath::Square(DeltaTime) / 2;

		//calculate the new acceleration
		const FVector NewAcceleration = CalculateAccel(RopePoint);

		//calculate the new velocity of the verlet point
		const FVector NewVelocity = RopePoint.Velocity + (RopePoint.Acceleration + NewAcceleration) * DeltaTime / 2;

		//check for collisions and update the rope point
		if (CheckForCollisions(RopePoint, NewPosition, RopePoint.GetWL(), NewVelocity, NewAcceleration))
		{
			//add the collision point to the array (if it's not already in the array)
			CollisionPoints.AddUnique(&RopePoint);
		}

		////set the position, velocity, and acceleration of the rope point
		//RopePoint.SetWL(NewPosition);
		//RopePoint.Velocity = NewVelocity;
		//RopePoint.Acceleration = NewAcceleration;
	}



	//enforce the constraints of the rope
	EnforceConstraints();

	////set the old locations of the rope points
	//SetRopeOldLocations(DeltaTime);
}

FVector URopeComponent::CalculateAccel(const FRopePoint& RopePoint) const
{
	//get the forces acting on the verlet point
	const FVector GravityAcceleration= { 0, 0, -9.81 * VerletGravityFactor};

	//calculate the drag force on the rope point
	const FVector DragForce = 0.5f * RopeDrag * (RopePoint.Velocity * RopePoint.Velocity);

	//calculate the drag acceleration on the rope point
	const FVector DragAcceleration = DragForce / RopeMass;

	//calculate the acceleration on the rope point
	const FVector Acceleration = GravityAcceleration - DragAcceleration;

	//return the acceleration
	return Acceleration;

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

FCollisionQueryParams URopeComponent::GetCollisionParams() const
{
	//setup collision parameters for traces and sweeps
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());

	////iterate throught the array of ignored classes
	//for (const TSubclassOf<AActor> IgnoredClass : IgnoredClasses)
	//{
	//	//setup an array of the actors to ignore
	//	TArray<AActor*> IgnoredActors;

	//	//get the actors of this class
	//	UGameplayStatics::GetAllActorsOfClass(GetWorld(), IgnoredClass, IgnoredActors);

	//	//add the ignored actors to the collision parameters' ignored actors
	//	CollisionParams.AddIgnoredActors(IgnoredActors);
	//}

	return CollisionParams;
}

void URopeComponent::CheckCollisionPoints()
{
	//get the collision parameters
	const FCollisionQueryParams CollisionParams = GetCollisionParams();

	//iterate through all the rope points
	for (int Index = 0; Index < RopePoints.Num() - 1; Index++)
	{
		//check if we're not at the first rope point and that we're a collision point
		if (Index != 0 && RopePoints[Index].bIsCollisionPoint)
		{
			//sweep from the previous rope point to the next rope point
			FHitResult Surrounding;
			//GetWorld()->SweepSingleByChannel(Surrounding, RopePoints[Index - 1].GetWL(), RopePoints[Index + 1].GetWL(), FQuat(), ECC_Visibility, FCollisionShape::MakeSphere(RopeRadius), CollisionParams);
			GetWorld()->LineTraceSingleByChannel(Surrounding, RopePoints[Index - 1].GetWL(), RopePoints[Index + 1].GetWL(), CollisionChannel, CollisionParams);
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
		}

		//check that we're not using verlet integration
		if (!bUseVerletIntegration)
		{
			//hit result to check for new rope points
			FHitResult Next;

			//sweep from the current rope point to the next rope point
			//GetWorld()->SweepSingleByChannel(Next, RopePoints[Index].GetWL(), RopePoints[Index + 1].GetWL(), FQuat(), CollisionChannel, FCollisionShape::MakeSphere(RopeRadius), CollisionParams);
			GetWorld()->LineTraceSingleByChannel(Next, RopePoints[Index].GetWL(), RopePoints[Index + 1].GetWL(), CollisionChannel, CollisionParams);


			//check for hits
			if (Next.IsValidBlockingHit())
			{
				//if we hit something, add a new rope point at the hit location if we're not too close to the last rope point
				if (FVector::Dist(RopePoints[Index].GetWL(), Next.Location) > MinCollisionPointSpacing && FVector::Dist(RopePoints[Index + 1].GetWL(), Next.Location) > MinCollisionPointSpacing)
				{
					////insert the new rope point at the hit location
					//RopePoints.Insert(Next.Location + Next.ImpactNormal * 10, Index + 1);

					//check if the hit actor has a grappleable component
					if (Next.GetActor()->FindComponentByClass<UGrappleableComponent>())
					{
						//get the grappleable component and check if it's valid
						if (UGrappleableComponent* LocGrappleableComponent = Next.GetActor()->FindComponentByClass<UGrappleableComponent>())
						{
							//broadcast the collision grapple event
							LocGrappleableComponent->OnCollisionGrapple(GetOwner(), Next);
						}
					}

					//insert the new rope point at the correct tarray index
					RopePoints.Insert(FRopePoint(Next), Index + 1);
				}

				//DrawDebugLine(GetWorld(), RopePoints[Index].GetWL(), RopePoints[Index + 1].GetWL(), FColor::Yellow, false, 0.f, 0, 5.f);
			}
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
			//check if we have an even index
			if (Index % 2 == 0)
			{
				//draw a debug line between the current rope point and the next rope point
				DrawDebugLine(GetWorld(), RopePoints[Index].GetWL(), RopePoints[Index + 1].GetWL(), FColor::Blue, false, 0.f, 0, 5.f);
			}
			else
			{
				//draw a debug line between the current rope point and the next rope point
				DrawDebugLine(GetWorld(), RopePoints[Index].GetWL(), RopePoints[Index + 1].GetWL(), FColor::Red, false, 0.f, 0, 5.f);
			}
		}

		//return to prevent further execution
		return;
	}

	//check if we don't have a valid Niagara system to render
	if (!NiagaraSystem->IsValidLowLevelFast())
	{
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

	//clear the rope points array
	RopePoints.Empty();

	//check if we're using verlet integration
	if (bUseVerletIntegration)
	{
		//clear the constraints array
		Constraints.Empty();
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef (non-const reference is required for the OtherActor parameter)
void URopeComponent::ActivateRope(const FHitResult& HitResult)
{
	//set the grappleable component
	this->GrappleableComponent = HitResult.GetActor()->FindComponentByClass<UGrappleableComponent>();

	//set the active state to true
	bIsRopeActive = true;

	//set the rope points
	RopePoints = { FRopePoint(GetOwner(), GetComponentLocation()), FRopePoint(HitResult) };
	RopePoints[0].Component = PlayerCharacter->RopeMesh;

	//get the direction from the first rope point to the second rope point
	const FVector Direction = RopePoints[1].GetWL() - RopePoints[0].GetWL();

	//check if we're using verlet integration
	if (bUseVerletIntegration)
	{
		//add the extra verlet points
		for (int Index = 0; Index < NumVerletPoints - 1; ++Index)
		{
			//get how far along the rope the verlet point should be
			const float Alpha = float(Index + 1) / float(NumVerletPoints + 1);

			//the position of the rope point interpolated between the two rope points
			const FVector Position = RopePoints[0].GetWL() + Direction * Alpha;

			//add the verlet point to the rope
			RopePoints.Insert(FRopePoint(Position), RopePoints.Num() - 1);

			////draw a debug sphere at the position of the verlet point
			//DrawDebugSphere(GetWorld(), Position, 5, 6, FColor::Red, false, 5.f, 0, 5.f);
		}

		//get the distance between of the constraint
		const float Dist = Direction.Size() / (NumVerletPoints + 1) * (1 - Stiffness);

		//add the constraints
		for (int Index = 0; Index < RopePoints.Num() - 1; ++Index)
		{
			//get how far along the rope the the constraint is
			const float Alpha = float(Index + 1) / float(NumVerletPoints + 1);

			//get the value of constraint compensation 1 curve
			const float Compensation1 = ConstraintCompensation1Curve->GetFloatValue(Alpha);

			//get the value of constraint compensation 2 curve
			const float Compensation2 = ConstraintCompensation2Curve->GetFloatValue(Alpha);

			//add the constraint to the rope
			Constraints.Add(FVerletConstraint(&RopePoints[Index], &RopePoints[Index + 1], Compensation1, Compensation2, Dist));

			////draw a debug sphere in the middle of the constraint
			//DrawDebugSphere(GetWorld(), RopePoints[Index].GetWL() + Direction * Alpha / 2, Dist / 2, 6, FColor::Green, false, 5.f, 0, 5.f);
		}
	}

	////set the old locations of the rope points
	//SetRopeOldLocations(GetWorld()->GetDeltaSeconds());
}

FVector URopeComponent::GetRopeDirection() const
{
	//get the direction from the first rope point to the second rope point
 	return (RopePoints[1].GetWL() - RopePoints[0].GetWL()).GetSafeNormal();
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
