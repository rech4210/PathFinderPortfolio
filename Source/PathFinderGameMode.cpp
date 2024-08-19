// Copyright Epic Games, Inc. All Rights Reserved.

#include "PathFinderGameMode.h"
#include "PathFinderCharacter.h"
#include "UObject/ConstructorHelpers.h"

APathFinderGameMode::APathFinderGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
