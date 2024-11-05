// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PFActor.generated.h"

class APFNode;
class APFinder;

// enum EPathfindingAlgorithm;
UCLASS()
class PATHFINDER_API APFActor : public AActor
{
	GENERATED_BODY()
public:
	APFActor();
	
	UPROPERTY()
	TObjectPtr<APFNode> DestinationNode;
};
