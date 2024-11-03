// Copyright Epic Games, Inc. All Rights Reserved.

#include "PathFinderGameMode.h"
#include "UObject/ConstructorHelpers.h"
// constexpr int MAX_NODE_COUNT = 5;

FActorSpawnParameters params;
FVector SpawnLocation = FVector::Zero();
FRotator SpawnRotator = FRotator::ZeroRotator;

APathFinderGameMode::APathFinderGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	CurrentWorld = GetWorld();
}

FName PFinderPath = TEXT("Blueprint'/Game/Blueprint/PathFinder/BP_Finder.BP_Finder'");
void APathFinderGameMode::BeginPlay() {
	Super::BeginPlay();
}


FName BP_PFActor = TEXT("Blueprint'/Game/Blueprint/Actor/BP_PFActor.BP_PFActor'");
// void APathFinderGameMode::GenerateMovingActor(FVector ActorLocation, AActor* TargetActor) {
// 	auto BPTempActor = CurrentWorld->SpawnActor<APFActor>(Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(),nullptr,*BP_PFActor.ToString()))->GeneratedClass,ActorLocation,SpawnRotator,params);
// 	auto BPTargetActor = CurrentWorld->SpawnActor<APFActor>(Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(),nullptr,*BP_PFActor.ToString()))->GeneratedClass,ActorLocation,SpawnRotator,params);
// }

