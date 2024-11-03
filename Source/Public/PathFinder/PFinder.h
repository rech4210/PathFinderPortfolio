// Fill out your copyright notice in the Description page of Project Settings.

#pragma once




#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UI/AlgorithmUI.h"
#include "PFinder.generated.h"

class APFNode;
class APFActor;

template<typename Element>
class FCustomPriorityQueue;

template<typename Element>
struct FCustomPriorityQueueNode;


DECLARE_DYNAMIC_DELEGATE(FFinderBFSButtonSet);
DECLARE_DYNAMIC_DELEGATE(FFinderDijkButtonSet);
DECLARE_DYNAMIC_DELEGATE(FFinderAstarButtonSet);
DECLARE_DYNAMIC_DELEGATE(FFinderJPSButtonSet);

static const FString guasa = TEXT("hello");
extern PATHFINDER_API FString gut;

UCLASS()
class PATHFINDER_API APFinder : public AActor
{
	GENERATED_BODY()
public:
	const FString gut = TEXT("heelo");
	APFinder();
	UPROPERTY(EditAnywhere,BlueprintReadWrite , Category ="Plain")
	TObjectPtr<AActor> Area = nullptr;
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite , Category ="Plain")
	TObjectPtr<AActor> SavedArea = nullptr;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category ="Node")
	TSubclassOf<APFNode> Node = nullptr;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category ="Node")
	APFNode* StartNode = nullptr;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category ="Node")
	APFNode* DestNode = nullptr;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category ="Node")
	APFNode* SavedNode = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Node")
	int NodeGap;
	UPROPERTY(BlueprintReadOnly, Category="Node")
	UShapeComponent* NodeCollision;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<APFNode*> NodeArray;
	UPROPERTY(BlueprintReadOnly)
	TArray<APFNode*> ReconstructedNodeArray;
	UPROPERTY(EditAnywhere, Category= "Debug")
	float DebugLineDuration = 3.0f;
	UPROPERTY(EditAnywhere, Category= "Debug")
	float DebugResultLineDuration = 15.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category ="Size")
	float MinX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category ="Size")
	float MinY;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category ="Size")
	float MaxX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category ="Size")
	float MaxY;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Node")
	int NodeDensity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category ="Size")
	float TopZ =.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	const TSubclassOf<UUserWidget> FinderUIClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAlgorithmUI * FinderUI = nullptr;
	
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
	void SetBound();
	UFUNCTION(BlueprintCallable)
	void DrawGrid();

	UPROPERTY(BlueprintReadWrite)
	FFinderBFSButtonSet BFS_Delegate;
	
	UPROPERTY(BlueprintReadWrite)
	FFinderDijkButtonSet Dijkstra_Delegate;
	
	UPROPERTY(BlueprintReadWrite)
	FFinderAstarButtonSet Astar_Delegate;

	UPROPERTY(BlueprintReadWrite)
	FFinderJPSButtonSet JPS_Delegate;
	

private:
	TMap<APFNode*, APFNode*> currentPath;
	UFUNCTION(BlueprintCallable)
	void BFS_Algorithm();
	UFUNCTION(BlueprintCallable)
	void Dijkstra_Algorithm();
	UFUNCTION(BlueprintCallable)
	void A_star_Algorithm();
	UFUNCTION(BlueprintCallable)
	void JPS_Algorithm();

	void ReconstructPath(const TMap<APFNode*,APFNode*>& Path, APFNode* LastSavedNode,FColor color = FColor::Red);
	static inline auto Heuristic(const APFNode* to, const APFNode* from) noexcept -> float; // for A*
};
