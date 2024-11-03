#include "Actor/PFNode.h"

APFNode::APFNode(){
	PrimaryActorTick.bCanEverTick = false;
}

void APFNode::BeginPlay(){
	Super::BeginPlay();
}

void APFNode::AddConnection(TArray<AActor*> nearstNodes) {
	for (auto node : nearstNodes) {
		if(Connection.Contains(node) || !IsValid(node)) continue;
		auto Apfnode = Cast<APFNode>(node);
		Connection.Add(Apfnode);
	}
	ConnectionCount = Connection.Num();
}

void APFNode::SetNodeCost(int cost) {
	NodeCost = cost;
}

TArray<APFNode*>& APFNode::GetConnectedNode() {
	return Connection;
}

