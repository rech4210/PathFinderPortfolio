// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/PFActor.h"


// Sets default values
APFActor::APFActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}


// Called every frame
void APFActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PathFinder->A_star_Algorithm(this,StartVector,ArrivalVector);
}

//
// void APFActor::FindPath(FVector3f current, FVector3f arrival) const {
// }
