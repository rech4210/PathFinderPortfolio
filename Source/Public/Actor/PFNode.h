#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "PFNode.generated.h"

UCLASS()
class PATHFINDER_API APFNode : public AActor
{
	GENERATED_BODY()
	
public:	
	APFNode();
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UShapeComponent* CollisionMesh = nullptr;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int ConnectionCount = 0;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float NodeGap = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float NodeCost = 0.0f;
	UPROPERTY()
	APFNode* NextNode = nullptr;

	UFUNCTION(BlueprintCallable)
	void AddConnection(TArray<AActor*> nearstNodes);
	UFUNCTION(BlueprintCallable)
	void SetNodeCost(int cost);
	UFUNCTION(BlueprintImplementableEvent)
	void SetNodeCunnection();
	UFUNCTION(BlueprintImplementableEvent)
	void OverlappedCheck(bool& Result);
	UFUNCTION()
	TArray<APFNode*>& GetConnectedNode();
protected:
	TArray<APFNode*> Connection;
	virtual void BeginPlay() override;
};
