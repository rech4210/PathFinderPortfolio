#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "AlgorithmUI.generated.h"
class APFNode;
class APFinder;

UCLASS()
class PATHFINDER_API UAlgorithmUI : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	APFinder* Finder;

	UPROPERTY(BlueprintReadWrite)
	UButton * BFS_button = nullptr;
	UPROPERTY(BlueprintReadWrite)
	UButton * Dijkstra_button = nullptr;
	UPROPERTY(BlueprintReadWrite)
	UButton * Astar_button = nullptr;
	UPROPERTY(BlueprintReadWrite)
	UButton * JPS_button = nullptr;
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void SetButtonData();
};
