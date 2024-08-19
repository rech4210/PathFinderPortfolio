// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavigationSystem.h"
#include "Interface/PFAlgorithm.h"
#include "PFCustomPathFinder.generated.h"
/**
 * 
 */
enum class EPathfindingAlgorithm
{
	Bfs,
	Dijkstra,
	AStar,
	Jsp
};

UCLASS(Blueprintable)
class PATHFINDER_API UPFCustomPathFinder : public UNavigationSystemV1/*, public IPFAlgorithm*/
{
	GENERATED_BODY()

public:
	UPROPERTY()
	AActor* PFActor;

	
	void CustomPathfinder(const AActor* StartActor,const AActor* EndActor, EPathfindingAlgorithm Algorithm);

	virtual FVector3f BFS_Algorithm(AActor* target, FVector3f current, FVector3f arrival);
	virtual FVector3f Dijkstra_Algorithm(AActor * target,FVector3f current, FVector3f arrival);
	virtual FVector3f A_star_Algorithm(AActor * target,FVector3f current, FVector3f arrival);
	virtual FVector3f JSP_Algorithm(AActor * target,FVector3f current, FVector3f arrival);
};
