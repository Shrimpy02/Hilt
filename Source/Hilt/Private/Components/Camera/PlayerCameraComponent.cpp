// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Camera/PlayerCameraComponent.h"

UPlayerCameraComponent::UPlayerCameraComponent()
{
}

void UPlayerCameraComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	////check if we have a valid world
	//if (!GetWorld()->IsValidLowLevelFast())
	//{
	//	//call the parent implementation
	//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//	//return
	//	return;
	//}

	////check that we're not in a preview world
	//if (GetWorld()->IsPreviewWorld())
	//{
	//	//call the parent implementation
	//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//	//return
	//	return;	
	//}

	////update the field of view
	//FieldOfView = InterpToTarget(CurrentCameraState.FovInterpType, FieldOfView, CurrentCameraState.FieldOfView, CurrentCameraState.FovInterpSpeed, DeltaTime);

	////update the post process settings
	//PostProcessSettings = CurrentCameraState.PostProcessSettings;

	//call the parent implementation
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
