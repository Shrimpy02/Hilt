// Class Includes
#include "InteractableObjects/LaunchPad.h"

// Other Includes
#include "Player/PlayerCharacter.h"
#include "Components/PlayerMovementComponent.h"
#include "Components/BoxComponent.h"

// ---------------------- Constructor`s -----------------------------
ALaunchPad::ALaunchPad()
{
	PrimaryActorTick.bCanEverTick = true;

	// Trigger Collision Mesh  -------------
	TriggerCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerCollisionBox"));
	TriggerCollisionBox->SetupAttachment(GetRootComponent());
	// Collision Settings
	TriggerCollisionBox->SetCollisionObjectType(ECC_WorldStatic);
	TriggerCollisionBox->SetGenerateOverlapEvents(true);
	TriggerCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TriggerCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	TriggerCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	TriggerCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

// ---------------------- Public Function`s -------------------------

void ALaunchPad::BeginPlay()
{
	Super::BeginPlay();
	TriggerCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ALaunchPad::OnOverlap);
}

void ALaunchPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALaunchPad::RemoveLevelPresence()
{
	Super::RemoveLevelPresence();

	// Trigger Collision Box mesh ------------
	// Disable collision
	TriggerCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void ALaunchPad::AddLevelPresence()
{
	Super::AddLevelPresence();

	// Reset cooldown
	GetWorld()->GetTimerManager().ClearTimer(MainTimerHandler);
	CooldownComplete();

	// Trigger Collision Box ------------
	// Enable collision
	TriggerCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ALaunchPad::ResetCooldown()
{
	// Reset cooldown
	GetWorld()->GetTimerManager().ClearTimer(MainTimerHandler);
	CooldownComplete();
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
		GetWorldTimerManager().SetTimer(MainTimerHandler, this, &ALaunchPad::CooldownComplete, LaunchPadCoolDownTime);
	}
}

// --------------------- Private Function`s -------------------------

FVector ALaunchPad::CalcThrowDirection()
{
	FRotator ActorRotation = GetActorRotation();
	return ActorRotation.RotateVector(RelativeThrowDirection);
}

void ALaunchPad::ThrowActor(AActor* _actor)
{
	if (_actor)
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(_actor);
		if (Player)
			Player->PlayerMovementComponent->AddImpulse(CalcThrowDirection() * DefaultThrowStrength, true);

		ThrewAnActor();
	}
}

void ALaunchPad::CooldownComplete()
{
	CoolingDown = false;
	ThrowCoolDownComplete();
}


// ---------------- Getter`s / Setter`s / Adder`s --------------------

