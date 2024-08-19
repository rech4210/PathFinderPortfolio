// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PathFinder/PFCustomPathFinder.h"
#include "PFActor.generated.h"


UCLASS()
class PATHFINDER_API APFActor : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FVector3f StartVector;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FVector3f ArrivalVector;
	
	// Sets default values for this actor's properties
	APFActor();
	virtual void Tick(float DeltaTime) override;


	// UFUNCTION(BlueprintCallable)
	// void FindPath(FVector3f current, FVector3f arrival) const;
	
	// UFUNCTION(BlueprintCallable)
	// void SetPath(FVector3f start, FVector3f arrival) const;

protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TObjectPtr<UPFCustomPathFinder> PathFinder;
	
private:
	UPROPERTY()
	FVector3f Current;

};
