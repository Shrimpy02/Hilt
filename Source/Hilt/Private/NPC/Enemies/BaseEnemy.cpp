// Class Includes
#include "NPC/Enemies/BaseEnemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Core/HiltTags.h"

// Other Includes
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

// ---------------------- Constructor`s -----------------------------
ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Niagara Comp -----------
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	SetRootComponent(NiagaraComp);

	// Visible Mesh -----
	VisibleMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VisibleMesh"));
	VisibleMesh->SetupAttachment(GetRootComponent());
		// Collision Settings
	VisibleMesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	VisibleMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	VisibleMesh->SetGenerateOverlapEvents(true);
	VisibleMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	VisibleMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	VisibleMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	// Collision Mesh ------
	CollisionMesh = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionMesh"));
	CollisionMesh->SetupAttachment(GetRootComponent());
		// Collision Settings
	CollisionMesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	CollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionMesh->SetGenerateOverlapEvents(true);
}

// ---------------------- Public Function`s -------------------------

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	// Gets the player character and sets it as the combat target
	if (UWorld* World = GetWorld())
		if (APlayerController* PC = World->GetFirstPlayerController())
			if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(PC->GetPawn()))
				SetCombatTarget(PlayerCharacter);
			else
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No player class found as CombatTarget"));

	// Enable overlap events for begin and end on collision capsule. 
	CollisionMesh->OnComponentBeginOverlap.AddDynamic(this, &ABaseEnemy::OnOverlap);
	CollisionMesh->OnComponentEndOverlap.AddDynamic(this, &ABaseEnemy::EndOverlap);

	// AddTags
	Tags.Add(HiltTags::EnemyTag);
	Tags.Add(HiltTags::EnemyAliveTag);
}

void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateVFXLocationRotation();
}

void ABaseEnemy::RemoveLevelPresence()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	ToggleActiveOrInactiveTag();
}

void ABaseEnemy::AddLevelPresence()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	ToggleActiveOrInactiveTag();
}

bool ABaseEnemy::IsAlive()
{
	return Tags.Contains(HiltTags::EnemyAliveTag) ? true : false;
}

float ABaseEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Health -= DamageAmount;
	if (Health <= 0 && CanDie)
		RemoveLevelPresence();
	
	return 0.0f;
}

bool ABaseEnemy::IsTargetPosWithinRange(const FVector& _targetLocation, const float _radiusFromSelfToCheck)
{
	float DistanceToTarget = GetDistanceBetweenTwoPoints(_targetLocation, GetActorLocation());

	return DistanceToTarget <= _radiusFromSelfToCheck;
}

void ABaseEnemy::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void ABaseEnemy::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void ABaseEnemy::UpdateVFXLocationRotation()
{
	if (NiagaraComp)
	{
		NiagaraComp->SetWorldLocation(GetActorLocation());
		NiagaraComp->SetWorldRotation(GetActorRotation());
	}
}

void ABaseEnemy::PlayVFX(UNiagaraSystem* _niagaraVFX, FVector _location, FRotator _rotation)
{
	if (_niagaraVFX && NiagaraComp) {
		NiagaraComp->SetAsset(_niagaraVFX);

		NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			_niagaraVFX,
			_location,
			_rotation,
			FVector(1.f),
			true,
			true,
			ENCPoolMethod::None,
			true);
	}
}

void ABaseEnemy::PlayAudio(USoundBase* _soundBase, FVector _location)
{
	if (_soundBase)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			_soundBase,
			_location
		);
	}
}

// --------------------- Private Function`s -------------------------

void ABaseEnemy::ToggleActiveOrInactiveTag()
{
	if (Tags.Contains(HiltTags::EnemyAliveTag) && !Tags.Contains(HiltTags::EnemyDeadTag))
	{
		Tags.Remove(HiltTags::EnemyAliveTag);
		Tags.Add(HiltTags::EnemyDeadTag);
	}
	else if (!Tags.Contains(HiltTags::EnemyAliveTag) && Tags.Contains(HiltTags::EnemyDeadTag))
	{
		Tags.Add(HiltTags::EnemyAliveTag);
		Tags.Remove(HiltTags::EnemyDeadTag);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(1, 10.f, FColor::Red, TEXT("Error: Duplicate active/inactive tags detected on enemy"));
	}
}

// ---------------- Getter`s / Setter`s / Adder`s --------------------

FVector ABaseEnemy::GetPointWithRotator(const FVector& Start, const FRotator& Rotation, float Distance)
{
	FQuat Quaternion = Rotation.Quaternion();
	FVector Direction = Quaternion.GetForwardVector();
	FVector Point = Start + (Direction * Distance);

	return Point;
}

FVector ABaseEnemy::GetForwardVectorOfRotation(const FRotator& Rotation)
{
	FQuat Quaternion = Rotation.Quaternion();
	FVector Direction = Quaternion.GetForwardVector();

	return Direction;
}

FVector ABaseEnemy::GetVectorBetweenTwoPoints(const FVector& Point1, const FVector& Point2)
{
	FVector VectorBetweenLocations = Point1 - Point2;
	VectorBetweenLocations.Normalize();

	return -VectorBetweenLocations;
}

float ABaseEnemy::GetDistanceBetweenTwoPoints(const FVector& Point1, const FVector& Point2)
{
	float DistanceToTarget = FVector::Dist(Point1, Point2);

	return DistanceToTarget;
}