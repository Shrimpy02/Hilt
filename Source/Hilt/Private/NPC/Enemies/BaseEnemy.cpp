// Class Includes
#include "NPC/Enemies/BaseEnemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"


// Other Includes
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

// ---------------------- Constructor`s -----------------------------
ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;


	// Collision Mesh
	CollisionMesh = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionMesh"));
	SetRootComponent(CollisionMesh);

		// Collision Settings
	CollisionMesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	CollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionMesh->SetGenerateOverlapEvents(true);
	CollisionMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Visible Mesh
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());

		// Collision Settings
	Mesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetGenerateOverlapEvents(true);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	// Niagara
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComp->SetupAttachment(GetRootComponent());
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
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No player class vound as CombatTarget"));

	// Enable overlap events for begin and end on collision capsule. 
	CollisionMesh->OnComponentBeginOverlap.AddDynamic(this, &ABaseEnemy::OnOverlap);
	CollisionMesh->OnComponentEndOverlap.AddDynamic(this, &ABaseEnemy::EndOverlap);
}

void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateVFXLocationRotation();
}

float ABaseEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Health -= DamageAmount;
	if (Health <= 0 && CanDie)
		Die();
	
	return 0.0f;
}

void ABaseEnemy::Die()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	this->Destroy();
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