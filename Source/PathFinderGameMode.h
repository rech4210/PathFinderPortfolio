// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PathFinderGameMode.generated.h"

class APFNode;
class APFinder;
class APFActor;

UCLASS(minimalapi)
class APathFinderGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APathFinderGameMode();
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TObjectPtr<APFinder> PathFinder;
	// UFUNCTION(BlueprintCallable)
	// void GenerateMovingActor(FVector ActorLocation ,AActor* TargetActor);

private:
	virtual void BeginPlay() override;

	TArray<APFActor*> ActorArray;
	UPROPERTY()
	TObjectPtr<APFActor> PlayerActor; // not used
	UPROPERTY()
	TObjectPtr<UWorld> CurrentWorld;
	UPROPERTY()
	TObjectPtr<APFNode> TargetNode;
	TSubclassOf<APFActor> BPPlayerPawn;
};



