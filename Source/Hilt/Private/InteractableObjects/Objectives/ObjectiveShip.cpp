// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableObjects/Objectives/ObjectiveShip.h"

// Sets default values
AObjectiveShip::AObjectiveShip()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AObjectiveShip::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AObjectiveShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

