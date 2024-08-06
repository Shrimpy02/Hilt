#pragma once
#include "CoreMinimal.h"

namespace HiltTags
{
	//Tag for actors that should not be grappled
	static FName NoGrappleTag = FName("NoGrapple");

	// Objective
	static FName ObjectiveTag = FName("Objective");

	// Objects
	static FName ObjectTag = FName("InteractableObject");
	static FName ObjectActiveTag = FName("ObjectActive");
	static FName ObjectNotActiveTag = FName("ObjectNotActive");

	// Enemies
	static FName EnemyTag = FName("Enemy");
	static FName EnemyAliveTag = FName("EnemyAlive");
	static FName EnemyDeadTag = FName("EnemyDead");
}
