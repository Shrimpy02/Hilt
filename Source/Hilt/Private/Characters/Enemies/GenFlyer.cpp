// Class Includes
#include "Characters/Enemies/GenFlyer.h"

// Other Includes

// ---------------------- Constructor`s -----------------------------
AGenFlyer::AGenFlyer()
{
	PrimaryActorTick.bCanEverTick = true;

}


// ---------------------- Public Function`s -------------------------

void AGenFlyer::BeginPlay()
{
	Super::BeginPlay();
	EnemyState = EEnemyState::EES_Patroling;
	CurrentPatrolTarget = ChoosePatrolTarget();
}

void AGenFlyer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CanMove == true)
		if(WaitingAtPatrolPointLocation == false)
			MoveToTarget(DeltaTime);
	
}

// --------------------- Private Function`s -------------------------

AActor* AGenFlyer::ChoosePatrolTarget()
{
	//Choose random patrol point
	if (RandomPatrolPointSelection == true)
	{
		TempPatrolTargets.Empty();
		for (AActor* Target : PatrolTargets)
		{
			if (Target != CurrentPatrolTarget)
			{
				TempPatrolTargets.AddUnique(Target);
			}
		}

		const int32 NumPatrolTargets = TempPatrolTargets.Num();
		if (NumPatrolTargets > 0)
		{
			const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);
			return TempPatrolTargets[TargetSelection];
		}
	}
	else // Choose next patrol point in order
	{
		const int32 NumPatrolTargets = PatrolTargets.Num();
		if (NumPatrolTargets > 0)
		{
			CurrentTargetIndex = (CurrentTargetIndex + 1) % NumPatrolTargets;
			return PatrolTargets[CurrentTargetIndex];
		}
	}

	return nullptr;
}

void AGenFlyer::CheckPatrolTarget()
{
	// If there is no patrol target end function
	if (!CurrentPatrolTarget) return;

	// If in range of patrol point
	if (IsTargetPosWithinRange(CurrentPatrolTarget->GetActorLocation(), PatrolPointReachedRange))
	{
		// Choose new target
		CurrentPatrolTarget = ChoosePatrolTarget();
		WaitingAtPatrolPointLocation = true;
		// Calls blueprint function for reached event
		ReachedPatrolPoint();
		// Sets a patrol timer
		const float WaitTime = FMath::RandRange(WaitAtPatrolLocationMin, WaitAtPatrolLocationMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AGenFlyer::PatrolTimerFinished, WaitTime);
	}

}

void AGenFlyer::PatrolTimerFinished()
{
	LeavingPatrolPoint();
	WaitingAtPatrolPointLocation = false;
}

bool AGenFlyer::RotateToFace(float DeltaTime, FVector Direction)
{
	FRotator StartRoatation = GetActorRotation();
	FRotator EndRoatation = Direction.Rotation();
	float Alpha = 1.f;

	FRotator InterpolatedRotation = FMath::Lerp(StartRoatation, EndRoatation, Alpha);
	SetActorRotation(InterpolatedRotation * DeltaTime);

	if (InterpolatedRotation == EndRoatation)
	{
		return true;
	}

	return false;
}

void AGenFlyer::MoveToTarget(float DeltaTime)
{
	if (CurrentPatrolTarget)
	{

		FVector Point1 = GetActorLocation();
		FVector Point2 = CurrentPatrolTarget->GetActorLocation();
		FVector MovementVector = GetVectorBetweenTwoPoints(Point1, Point2);

		FRotator CurrentRotation = GetActorRotation();
		FRotator TargetRotation = MovementVector.Rotation();

		FRotator NewRotation = FMath::Lerp(CurrentRotation, TargetRotation, RotationSpeed);
		SetActorRotation(NewRotation);

		if (CurrentRotation.Equals(TargetRotation, 5.f))
		{
			FVector NewLocation = GetActorLocation();
			NewLocation += MovementVector * MovementSpeed * DeltaTime;

			SetActorLocation(NewLocation);
		}

	}
}

// ---------------- Getter`s / Setter`s / Adder`s --------------------
