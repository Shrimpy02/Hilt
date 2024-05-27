// Class Includes
#include "InteractableObjects/LaunchPad.h"

// Other Includes
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Player/PlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

// ---------------------- Constructor`s -----------------------------
ALaunchPad::ALaunchPad()
{
	PrimaryActorTick.bCanEverTick = true;


	// Collision Mesh
	CollisionMesh = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionMesh"));
	SetRootComponent(CollisionMesh);

	// Collision Settings
	CollisionMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	CollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionMesh->SetGenerateOverlapEvents(true);
	CollisionMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

	// Visible Mesh
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());

	// Collision Settings
	Mesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetGenerateOverlapEvents(true);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	// Launch Mesh
	LaunchMesh = CreateDefaultSubobject<USphereComponent>(TEXT("LaunchMesh"));
	LaunchMesh->SetupAttachment(GetRootComponent());

		// Collision Settings
	LaunchMesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	LaunchMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	LaunchMesh->SetGenerateOverlapEvents(true);
	LaunchMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	LaunchMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	LaunchMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Niagara
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComp->SetupAttachment(GetRootComponent());
}

// ---------------------- Public Function`s -------------------------

void ALaunchPad::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALaunchPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateVFXLocationRotation();
}

void ALaunchPad::ThrowActor(AActor* _actor)
{
	if(_actor)
	{
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(_actor))
			GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Green, TEXT("Throw Player"));
		
		else
			GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Yellow, TEXT("Throw Random Actor"));
		ThrewAnActor();
	}
		
	// How it used to be:
	//Player->PlayerMovementComponent->AddImpulse(ThrowAngle * _throwPower);
	//PlayVFXActivate(GetActorLocation());
	//PlayAudioActivate(GetActorLocation());
}

void ALaunchPad::CooldownComplete()
{
	CoolingDown = false;
	ThrowCoolDownComplete();
}

void ALaunchPad::UpdateVFXLocationRotation()
{
	if (NiagaraComp)
	{
		NiagaraComp->SetWorldLocation(GetActorLocation());
		NiagaraComp->SetWorldRotation(GetActorRotation());
	}
}

void ALaunchPad::PlayVFX(UNiagaraSystem* _niagaraVFX, FVector _location, FRotator _rotation)
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

void ALaunchPad::PlayAudio(USoundBase* _soundBase, FVector _location)
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

void ALaunchPad::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (CoolingDown == false)
	{
		ThrowActor(OtherActor);
		// Sets jumped bool so that function does not repeat.
		CoolingDown = true;
		// Resets Jumped to false when x seconds has gone. 
		GetWorldTimerManager().SetTimer(JumpResetTimerHandeler, this, &ALaunchPad::CooldownComplete, LaunchPadCoolDownTime);
	}
}

void ALaunchPad::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

// --------------------- Private Function`s -------------------------



// ---------------- Getter`s / Setter`s / Adder`s --------------------
