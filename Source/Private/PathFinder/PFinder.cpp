#include "PathFinder/PFinder.h"
#include "Actor/PFNode.h"
#include "Components/SphereComponent.h"
#include "Containers/Queue.h"

DECLARE_STATS_GROUP(TEXT("PathFind"), STATGROUP_PathFind,STATGROUP_Advanced);
DECLARE_CYCLE_STAT(TEXT("BFS"),STAT_BFS, STATGROUP_PathFind);
DECLARE_CYCLE_STAT(TEXT("Dijkstra"),STAT_Dijkstra, STATGROUP_PathFind);
DECLARE_CYCLE_STAT(TEXT("Astar"),STAT_Astar, STATGROUP_PathFind);

DECLARE_STATS_GROUP(TEXT("PathFindMemory"), STATGROUP_PathFindMemory, STATCAT_Advanced);
DECLARE_MEMORY_STAT(TEXT("NodeArray(Kb)"),STAT_MEMORY_NodeArray, STATGROUP_PathFindMemory);
DECLARE_MEMORY_STAT(TEXT("SearchedPath(Kb)"),STAT_MEMORY_SearchedPath, STATGROUP_PathFindMemory);

template<typename ElementType>
struct FCustomPriorityQueueNode {
	ElementType Element;
	float Priority;

	FCustomPriorityQueueNode(){Priority =0.f;}
	
	FCustomPriorityQueueNode(ElementType InElement, float Inpriority) {
		Element = InElement;
		Priority = Inpriority;
	}

	bool operator<(const FCustomPriorityQueueNode<ElementType> Other) const {
		return Priority < Other.Priority;
	}
};

template<typename ElementType>
class FCustomPriorityQueue {
public:
	FCustomPriorityQueue() {
		Array.Heapify();
	}

	ElementType Pop() {
		if(IsEmpty()) return nullptr;
		FCustomPriorityQueueNode<ElementType> Node;
		Array.HeapPop(Node);
		return Node.Element;
	}
	FCustomPriorityQueueNode<ElementType> PopNode() {
		FCustomPriorityQueueNode<ElementType> Node;
		Array.HeapPop(Node);
		return Node;
	}

	void Push(ElementType Node, float Priority){
		if(Array.Num() > 100000) {return;}
		Array.HeapPush(FCustomPriorityQueueNode<ElementType>(Node,Priority));
	}
	
	bool IsEmpty() noexcept {
		return Array.IsEmpty();
	}
	
private:
	TArray<FCustomPriorityQueueNode<ElementType>> Array;
};

FName BP_NodePath = TEXT("Blueprint'/Game/Blueprint/Actor/BP_Node.BP_Node'");
APFinder::APFinder(): MinX(.0f), MinY(.0f), MaxX(.0f), MaxY(.0f), NodeDensity(3){
	Node = Cast<UClass>(StaticLoadObject(UClass::StaticClass(),nullptr,*BP_NodePath.ToString()));
	BFS_Delegate.BindDynamic(this,&APFinder::BFS_Algorithm);
	Dijkstra_Delegate.BindDynamic(this,&APFinder::Dijkstra_Algorithm);
	Astar_Delegate.BindDynamic(this,&APFinder::A_star_Algorithm);
	JPS_Delegate.BindDynamic(this,&APFinder::JPS_Algorithm);
}

void APFinder::BeginPlay() {
	Super::BeginPlay();
} 

FActorSpawnParameters defaultParameter;

void APFinder::DrawGrid() {
	TArray<APFNode*> tempArray;
	float boundX = MaxX - MinX;
	float CurrentY = MinY;
	NodeGap = boundX / NodeDensity;
	while (CurrentY < MaxY) {
		float CurrentX = MinX;
		while (CurrentX < MaxX) {
			FTransform Transform = FTransform(FVector(CurrentX,CurrentY,TopZ));
			APFNode* tempNode = Cast<APFNode>(GetWorld()->SpawnActor(Node,&Transform,defaultParameter));
			tempNode->NodeGap = NodeGap;
			bool overlapped = false;
			
			tempNode->OverlappedCheck(overlapped);
			if(overlapped){
				tempNode->Destroy();
				CurrentX += NodeGap;
				continue;
			}

			tempArray.Add(tempNode);
			CurrentX += NodeGap;
		}
		CurrentY += NodeGap;
	}
	
	for (APFNode* tempNode : tempArray) {
		NodeCollision = tempNode->CollisionMesh;
		NodeCollision->SetActive(false);

		if(auto shpere = Cast<USphereComponent>(NodeCollision)) {
			shpere->SetSphereRadius((NodeGap*0.5f)+5);
		}
		if(auto box = Cast<UBoxComponent>(NodeCollision)) {
			box->SetLineThickness(4.0f);
			box->SetBoxExtent(FVector(NodeGap-5,NodeGap-5,15.0f),true);
		}
		NodeCollision->ShapeColor = FColor(47,244,10);
		NodeCollision->SetActive(true);
	}
	NodeArray = std::move(tempArray);
	
	int64 MemoryUsed = NodeArray.GetAllocatedSize();
	
	SET_MEMORY_STAT(STAT_MEMORY_NodeArray,MemoryUsed * 1024.f);
	for (auto node : NodeArray) {
		node->SetNodeCunnection();
	}
}

void APFinder::BFS_Algorithm() {
	SCOPE_CYCLE_COUNTER(STAT_BFS)
	if(!currentPath.IsEmpty()) currentPath.Reset();

	if(!(StartNode||DestNode)) {
		UE_LOG(LogTemp,Log,TEXT("Node not Setted"));
		return;
	}
	TMap<APFNode*, APFNode*> searchedPath;

	searchedPath.Add(StartNode,nullptr);
	TQueue<APFNode*> frontier;
	frontier.Enqueue(StartNode);

	while (!frontier.IsEmpty()) {
		APFNode* current = *frontier.Peek();
		frontier.Pop();

		for (APFNode* neighborNode : (current->GetConnectedNode())) {
			if(!searchedPath.Contains(neighborNode)) {
				searchedPath.Add(neighborNode, current);
				frontier.Enqueue(neighborNode);

				ReconstructPath(searchedPath, neighborNode);
				
				if(neighborNode == DestNode) {
					currentPath = MoveTemp(searchedPath);
					int64 memoryUsed = currentPath.GetAllocatedSize();
					SET_MEMORY_STAT(STAT_MEMORY_SearchedPath,memoryUsed * 1024.f);
					ReconstructPath(currentPath,DestNode, FColor::Cyan);
					return;
				}
			}
		}
	}
	UE_LOG(LogTemp,Log,TEXT("No Path"));
}

// 1. 우선순위 큐 최적화:
//  - 현재 TArray.Heapify를 사용하고 있는데, 이를 std::priority_queue나 boost::heap::fibonacci_heap으로 대체하면 성능이 향상될 수 있습니다.
//  - fibonacci_heap은 이론적으로 더 나은 시간 복잡도를 가지지만, 실제 상황에서는 std::priority_queue가 더 효율적일 수 있으므로 벤치마킹이 필요합니다.
//
// 2. 메모리 사용 최적화:
//  - SearchedPath와 PathCost를 TMap 대신 TArray로 변경하고, 노드 인덱스를 키로 사용하면 메모리 사용량과 접근 속도를 개선할 수 있습니다.
//
// 3. 조기 종료 조건:
// 4. 양방향 검색:
//  - 시작점과 목적지에서 동시에 검색을 시작하여 중간에서 만나는 지점을 찾는 방식으로 변경하면 검색 공간을 줄일 수 있습니다.
//
// 6. 캐싱 및 사전 계산:
//  - 자주 사용되는 경로를 캐싱하거나, 중요한 노드 간의 최단 경로를 사전에 계산해 두면 반복적인 계산을 줄일 수 있습니다.

void APFinder::Dijkstra_Algorithm() {
	SCOPE_CYCLE_COUNTER(STAT_Dijkstra)

	if(!currentPath.IsEmpty()) currentPath.Reset();
	
	TMap<APFNode*, APFNode*> searchedPath;
	TMap<APFNode*,double> pathCost;
	FCustomPriorityQueue<APFNode*> frontier;
	frontier.Push(StartNode, StartNode->NodeCost);
	searchedPath.Add(StartNode, nullptr);
	pathCost.Add(StartNode,StartNode->NodeCost);
	
	while (!frontier.IsEmpty()) {
		APFNode* current = frontier.Pop();
		if(current == DestNode) {
			UE_LOG(LogTemp,Log,TEXT("Dijkstra Total Node Cost : %f"),pathCost[DestNode]);
			currentPath = MoveTemp(searchedPath);
			int64 MemoryUsed = currentPath.GetAllocatedSize();
			SET_MEMORY_STAT(STAT_MEMORY_SearchedPath,MemoryUsed * 1024.f);

			ReconstructPath(currentPath,DestNode,FColor::Magenta);
			return;
		}
		
		for (auto neighborNode : current->GetConnectedNode()) {
			double newCost = neighborNode->NodeCost + pathCost[current];
			if(!searchedPath.Find(neighborNode) || pathCost[neighborNode] > newCost) {
				// SavedNode = neighborNode;
				searchedPath.Add(neighborNode, current);
				pathCost.Add(neighborNode,newCost);
				frontier.Push(neighborNode, newCost);
				ReconstructPath(searchedPath,neighborNode);

			}
		}
	}
}

void APFinder::A_star_Algorithm() {
	SCOPE_CYCLE_COUNTER(STAT_Astar)

	if(!currentPath.IsEmpty()) currentPath.Reset();
	TMap<APFNode*, APFNode*> searchedPath;
	TMap<APFNode*,double> pathCost;

	FCustomPriorityQueue<APFNode*> frontier; 
	frontier.Push(StartNode, StartNode->NodeCost);
	searchedPath.Add(StartNode, nullptr);
	pathCost.Add(StartNode,StartNode->NodeCost);
	
	while (!frontier.IsEmpty()) {
		APFNode* current = frontier.Pop();
		if(current == DestNode) {
			UE_LOG(LogTemp,Log,TEXT("Astar Total Node Cost : %f"),pathCost[DestNode]);
			currentPath = MoveTemp(searchedPath);
			int64 MemoryUsed = currentPath.GetAllocatedSize();
			SET_MEMORY_STAT(STAT_MEMORY_SearchedPath,MemoryUsed * 1024.f);
			ReconstructPath(currentPath,DestNode,FColor::Yellow);
			return;
		}
		for (auto neighborNode : current->GetConnectedNode()) {
			double newCost = neighborNode->NodeCost + pathCost[current];
			if(!searchedPath.Find(neighborNode) || pathCost[neighborNode] > newCost) {
				searchedPath.Add(neighborNode, current);
				pathCost.Add(neighborNode,newCost);
				double priority = newCost + Heuristic(neighborNode, DestNode);
				frontier.Push(neighborNode, priority);
				ReconstructPath(searchedPath, neighborNode);
			}
		}
	}
}


void APFinder::JPS_Algorithm() {
	UE_LOG(LogTemp, Log, TEXT("JPS"));
	TMap<APFNode*, APFNode*> searchedPath;
	currentPath = MoveTemp(searchedPath);
}

void APFinder::ReconstructPath(const TMap<APFNode*, APFNode*>& Path, APFNode* LastSavedNode,FColor color) {
	if(!LastSavedNode) return;
	APFNode* tempNode;
	if(color == FColor::Red) {
		tempNode = LastSavedNode;
		while (APFNode* valuePair = Path[tempNode]) {
			DrawDebugLine(GetWorld(), tempNode->GetActorLocation(), valuePair->GetActorLocation(), color,false,DebugLineDuration,0,10.0f);
			tempNode = valuePair;
		}
	}
	else {
		ReconstructedNodeArray.Reset();
		tempNode = DestNode;
		ReconstructedNodeArray.Push(tempNode);
		while (APFNode* valuePair = Path[tempNode]) {
			ReconstructedNodeArray.Push(valuePair);
			DrawDebugLine(GetWorld(), tempNode->GetActorLocation(), valuePair->GetActorLocation(), color,false,DebugResultLineDuration,0,15.0f);
			tempNode = valuePair;
		}
	}
	tempNode = nullptr;
	delete tempNode;
}


auto APFinder::Heuristic(const APFNode* to, const APFNode* from) noexcept -> float {
	return FVector::Dist(to->GetActorLocation(),from->GetActorLocation());
}
