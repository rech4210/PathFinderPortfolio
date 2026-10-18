# PathFinder

| 항목 | 내용 |
|:---|:---|
| 한 줄 요약 | 노드 기반 탐색 영역을 구성하고 BFS, Dijkstra, A* 길찾기를 구현·비교한 프로젝트 |
| 기간 | 2024.08 – 2024.11 |
| 구성 | 개인 프로젝트 |
| 스택 | Unreal Engine 5.3, C++, BFS/Dijkstra/A*, 메모리 프로파일링 도구 |
| 한계 | 지형 변화가 잦은 경우 유연한 대응이 어려울 수 있음 |

실행·재현: Unreal Engine 5.3에서 `PathFinder.uproject`를 열고 `APFinder`의 Blueprint 호출 함수로 `DrawGrid`와 길찾기 알고리즘을 실행한다.

## 선이 보이는 커스텀 길찾기 구현
사용 기술 : 언리얼5.3, C++, (BFS,Dijkstra,A*), 메모리 프로파일링 도구

개인 프로젝트
![image](https://github.com/user-attachments/assets/dd832468-0477-451c-870b-a5c5a70ca771)



길찾기는 NPC부터 캐릭터의 자동이동 구현과 적의 추적 시스템에 필수적인 기능이다. 길찾기 방법을 구현해보며, 길찾기의 성능 이슈를 확인해보며 이러한 문제가 생기는 원인을 분석하고 해결하는 방법을 찾아볼것이다. 나아가 효율적인 길찾기 방법을 구현하고 이를 적용해보는 것을 목표로 잡았다.


위와 같은 문제 원인을 파악하려면 우선 BFS, Dijkstra, A* `알고리즘`을 알 필요가 있다. 
다음으로 `탐색 영역`을 어떻게 지정할 것 인지에 대한 문제다. 당연히 탐색 불가능한 영역은 예외처리를 해야할 것이며, 지형에 대응한 길찾기를 수행하려면 탐색 영역 지정이 필수다.

탐색 영역 참고를 위해 언리얼 `navmesh`를 참고해보자.


![image](https://github.com/user-attachments/assets/95b66373-e9b1-42bc-8cf9-a9262f50ff4c)

▲ 언리얼 navmesh 시스템

Navmesh 시스템은 레밸 내의 충돌 지오메트리 기반으로 다각형 영역을 정의한다. 해당 영역에 대한 효율적인 처리를 위해 타일을 분할하여 개별적으로 로드하거나 업데이트 할 수 있다고 한다.


||장점|단점|
|---|---|---|
||다각형 메쉬로 구현되어 경로 탐색과 효율적 계산이 가능함|많은 동적 오브젝트가 존재할 시 업데이트가 빈번하게 필요해 성능 저하가 발생|
||레벨이 커짐에 따라 타일 단위로 관리하여 메모리 사용과 성능을 최적화 할 수 있음|너무 작은 타일 크기나 과도한 세분화는 품질을 저하시키며, 경로 탐색의 정확도에 영향을 주게 됨|
||지형 및 오브젝트 변화에 따라 동적으로 업데이트 가능|정적 NavMesh는 레벨 로딩 시에만 생성되며, 런타임 중에 변경이 어려움|


### 언리얼 NavMesh의 스캔
언리얼 NavMesh는 레벨의 충돌 지오메트리를 바탕으로 경로 탐색용 내비게이션 메시를 구성한다. 세부 내용은 [언리얼 엔진 Navigation System 공식 문서](https://dev.epicgames.com/documentation/unreal-engine/navigation-system-in-unreal-engine?lang=en-US)를 참고한다.

### 언리얼 NavMesh의 공간 분할 방법 (옥트리 타일 분할)

언리얼 Navigation System의 `FNavigationOctree`는 옥트리로 요소를 관리하고 추가·갱신·삭제 기능을 제공한다. 자세한 내용은 [FNavigationOctree 공식 API 문서](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/NavigationSystem/FNavigationOctree)를 참고한다.

NavMesh에서는 충돌 콜리전 데이터를 기반으로 스캔 mesh를 생성한다. 해당 mesh는 bake 되는 과정이 필요하므로 런타임 내 성능이 높으나 bake 시간이 필요하게 되며 mesh의 동적 변화에 대응하기 어렵다.

레벨에 런타임 액터를 배치해보는 것을 생각해보았다. 지정 영역에 배치할 시 동적으로 탐색할 수 있으며 런타임에 배치 간격, 탐색 범위를 직접 처리할 수 있다. 다만 다량의 액터를 배치하게되므로 이는 곧 런타임 성능 부하를 유발할 수 있기에 런타임 성능을 고려해보아야한다.

노드 액터 기반으로 라인을 그려 탐색 경로 시각화도 고려하였기에 노드 위치를 기반으로 탐색 수행하도록 결정하였다.

## 노드 배치 방법 

![image](https://github.com/user-attachments/assets/d8275096-e9ef-45e5-8610-de80a7cbc245)

노드 배치에 필요한 기능은 아래와 같다
- 탐색 영역 지정
- 탐색 간격 설정
- 노드 생성과 예외 조건 (벽 또는 탐색 불가 영역 지정)
- 인접 노드 탐색 방법

```cpp
void APFinder::DrawGrid() {
#Property
	TArray<APFNode*> tempArray;
	float boundX = MaxX - MinX;
	float CurrentY = MinY;
	NodeGap = boundX / NodeDensity;
	
#Set Node To Level 간격 별로 패치
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
	
#Set Node Actor CollisionMesh 충돌 컴포넌트 설정
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
	
# Make Connection Near Node Actor 인접 노드 저장
	for (auto node : NodeArray) {
		node->SetNodeCunnection();
	}
}
```
---

#### 구현 과정

- 탐색 영역 지정 및 간격 설정
탐색 영역을 설정하는데 두 가지 경우를 생각하였다. 십자 격자 기준으로 탐색하는 경우와 `대각선 탐색`까지 고려한 충돌 컴퍼넌트를 설정하여 4방향, 8방향의 탐색을 구현할 필요가 있었다. 해당 과정에서 Begin play의 틱이 과도한 충돌처리 발생으로 인해 프레임 드랍이 생기게 되었다. 이 부분을 보안하기위해 노드를 생성시킨 후 순회를 돌며 순차적으로 충돌 처리를 수행시켜주었다.
	- 탐색 영역을 Area로 설정하고 액터의 Bound 기준으로 프로퍼티를 설정
	- ![image](https://github.com/user-attachments/assets/4b013e95-636d-4ea8-825e-b6e88aa8aaf5)
	- ![image](https://github.com/user-attachments/assets/dada180e-b43e-469b-8911-5774955cb276)
> 노드 충돌 컴포넌트와 연결된 노드의 상태

- 노드 생성과 예외 조건
지형에 따른 노드 생성 예외처리가 필요했다. (언덕 지형 경사에 따른 위치 보정, 메쉬 내부에 생성되는 노드 객체의 위치 보정) 기존 언리얼 NavMesh는 충돌 콜리전을 기반으로 영역 스캔을 진행하지만, 해당 프로젝트에서는 이를 노드형식으로 제공하고 있다. 이에 따라 노드의 생성 위치를 표면적에 부착하기위해 충돌지점을 파악하고 충돌 된 위치에 재배치가 필요했다.
	- 충돌 컴퍼넌트를 기준으로 Tag를 비교하여 예외 처리를 수행하였음 경사진 언덕의 경우 충돌되는 접촉면을 기준으로 위치를 설정함
	- ![image](https://github.com/user-attachments/assets/14a63e31-2848-49b3-8742-44846ff25ec2)
	- ![image](https://github.com/user-attachments/assets/93cdeaec-5ea3-4cb4-a0b6-9b374093e408)
	- ![image](https://github.com/user-attachments/assets/d71e3711-a555-4c00-a3ad-26a605c0f888) 
	 > 길찾기 정확도를 위해 노드 밀집도를 고려하여야 했다. 밀집도는 메모리 사용량에 영향을 주며 탐색 횟수가 증가하기에 이에 대한 프로파일링이 필요했다.
	 - ![25](https://github.com/user-attachments/assets/1f2e688d-2657-4db6-9148-25e2bb8c7e09)
	 
	노드 밀집도 25 : 메모리 6.38KB
	 - ![40](https://github.com/user-attachments/assets/226cf574-499f-415a-ba30-04859c2fb9d5)
 
	노드 밀집도 40 : 메모리 17.14KB

	>README에 기록한 값 기준으로 밀집도 25는 6.38KB, 40은 17.14KB를 사용한다. 해당 레벨에서는 밀집도 40까지의 정확도는 필요없다고 판단된다. 따라서 밀집도 25를 선택하기로 했다.

<br>


- 인접 노드 탐색 방법
	- Actor 개체가 가진 충돌 컴퍼넌트를 기준으로 인접한 Node 객체를 탐색한다 (탐색 대상 특정을 위해 TSubClassof를 사용함)
	- ![image](https://github.com/user-attachments/assets/ed58d55a-bd8e-4e2d-b6fb-8f5245bfe498)

## 길찾기 탐색 방법

### 시각적 디버깅 이용
길찾기 알고리즘을 수행하고 결과 피드백을 위해 시각적 디버깅이 필요하다고 판단했다. 

![image](https://github.com/user-attachments/assets/a9dd3751-f21c-493c-a380-5bd2fb201552)


#### 패스를 재구성한 시각적 디버깅
```cpp
void APFinder::ReconstructPath(const TMap<APFNode*, APFNode*>& Path,
 APFNode* LastSavedNode,FColor color) {
	APFNode* tempNode = DestNode;
	ReconstructedNodeArray.Push(tempNode);
	#a. Path map을 기반으로 디버그 라인을 그린다. nullptr일 경우 종료
	while (APFNode* valuePair = Path[tempNode]) {
		ReconstructedNodeArray.Push(valuePair);
		DrawDebugLine(GetWorld(), tempNode->GetActorLocation(), valuePair->GetActorLocation(),
		 color,false,DebugResultLineDuration,0,15.0f);
		tempNode = valuePair;
	}
	tempNode = nullptr;
	delete tempNode;
}
```

길찾기 알고리즘이 수행되면 `ReconstructPath` 함수가 실행되며 Path에 저장된 노드의 역순 탐색을 진행한다. 

### BFS

![image](https://github.com/user-attachments/assets/5007b2c4-9b93-41f7-8d42-65b9487cd756)

&nbsp;&nbsp;&nbsp;*BFS*는 인접노드 우선 탐색의 규칙을 가진다. 탐색하려는 노드를 기준으로 인접한 노드를 순서대로 탐색하며 전체 영역을 탐색하게 되며 early exit를 통해 노드에 닿으면 탐색을 중단하게 하여 성능을 높일 수 있다.

최단 경로를 보장하는 장점이 존재하나, 넓은 범위를 탐색하게 되므로 (우선순위가 없는 탐색방법이기에) 메모리를 많이 사용하게 되며 탐색 속도가 느리다.


![image](https://github.com/user-attachments/assets/65db4d36-9d5e-4099-9e7e-a8c4fa67473d)


1. 시작 노드를 frontier에 넣는다
2. 탐색 대상 노드에 연결된 인접 노드들 (neighbor) 를 순차적으로 탐색한다. 이는 frontier에 노드가 없을 때 까지 반복한다.
	2-1. 만약 탐색되지않은 노드라면 저장한다  
3. 목표 노드에 도달하게 되면 유효한 경로만 포함해 출력한다.

```cpp
#BFS

void APFinder::BFS_Algorithm() {
#a 탐색된 노드를 저장하는 map
	TMap<APFNode*, APFNode*> searchedPath;
	TQueue<APFNode*> frontier;
	
	searchedPath.Add(StartNode,nullptr);
	frontier.Enqueue(StartNode);

	while (!frontier.IsEmpty()) {
		APFNode* current = *frontier.Peek();
		frontier.Pop();
		#b 인접 노드를 순회하며 경로 탐색
		for (APFNode* neighborNode : (current->GetConnectedNode())) {
			if(!searchedPath.Contains(neighborNode)) {
				searchedPath.Add(neighborNode, current);
				frontier.Enqueue(neighborNode);
				ReconstructPath(searchedPath, neighborNode);
				#c 목표 노드 탐색 완료시 early exit
				if(neighborNode == DestNode) {
					currentPath = MoveTemp(searchedPath);
					ReconstructPath(currentPath,DestNode, FColor::Cyan);
					return;
				}
			}
		}
	}
	UE_LOG(LogTemp,Log,TEXT("No Path"));
}

```


![BFS 프로파일링](https://github.com/user-attachments/assets/9af94c88-a476-4b76-8b50-6451fe774aa1)
> SearchedPath의 메모리는 21.22kb로 공간 전체를 탐색함에 따라 메모리 사용량이 많아지게 된다.

#### Early Exit의 경우

![image](https://github.com/user-attachments/assets/c7f45711-812d-4b67-b645-5bf10b584c63)
> 도착 노드 조건에 따라 early exit를 하게 될 경우 속도와 메모리 측면에서 이점을 얻을 수 있다.

최초 탐색 21.22 kb와 Early Exit를 적용한 경우 3.31 kb로 약 6배의 메모리 사용을 절감할 수 있게 되었다.

단순 길찾기 구현이라면 BFS는 간단한 로직으로 만들 수 있으므로 실용적일 것이나, 특정 조건에 따른 길찾기 수행을 위해선 다른 길찾기 방법을 사용해야한다.

---

### Dijkstra
![image](https://github.com/user-attachments/assets/1bfa8687-b86f-43bf-9f68-1806c9200906)

&nbsp;&nbsp; *Dijkstra*는 BFS 탐색에서 `가중치`의 개념이 포함된 길찾기 방법이다.  노드간의 간선(엣지)에 포함된 가중치 값으로 표현된다.

레벨의 지형에 따라 사막이 존재하거나 호수가 존재할 수 있다. 이러한 경우 지형에 따른 차이가 생기기 마련이며, 이를 가중치로 표현하여 지형을 우회이동할 수 있다.

---

### 우선순위큐 구현
Dijkstra 탐색을 위해선 가중치 기반 우선순위를 매겨야한다. 이를 위해선 우선순위큐를 구현해야한다. Stl에서는 priority_queue가 존재하지만 unreal에서 사용하기 위해 직접 구현할 필요가 있다.

```cpp
#a. 우선순위큐 노드
template<typename ElementType>
struct FCustomPriorityQueueNode {
	ElementType Element;
	float Priority;
	// 생성자....
	
	#b. < operator를 통해 우선순위를 결정하게된다
	bool operator<(const FCustomPriorityQueueNode<ElementType> Other) const {
		return Priority < Other.Priority;
	}
};

#c. 우선순위큐 내부는 Array.Heapify를 통해 힙으로 구현된다.
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
	#d. TArray의 바운드는 INT_MAX로 임의 바운드를 설정해준다.
		if(Array.Num() > 100000) {return;}
		Array.HeapPush(FCustomPriorityQueueNode<ElementType>(Node,Priority));
	}
	
	bool IsEmpty() noexcept {
		return Array.IsEmpty();
	}
	
private:
	TArray<FCustomPriorityQueueNode<ElementType>> Array;
};
```
<br>

우선순위큐를 구현하였으니 다음은 Dijkstra를 실행하면 된다.
Dijkstra는 아래 순서에 따라 실행된다.
1. 시작 노드를 frontier에 넣는다
2. 탐색 대상 노드의 인접한 노드들에 탐색 여부, 가중치와 현재 가중치를 비교하여 탐색한다.
3. 도착지점 도달시 유효 경로만 포함하여 출력한다.

---

```cpp
#Dijkstra
void APFinder::Dijkstra_Algorithm() {
	#a 노드-가중치 값을 저장하는 map
	TMap<APFNode*,double> pathCost;
	// property.....
	
	while (!frontier.IsEmpty()) {
		APFNode* current = frontier.Pop();
		#b 도착지점 도달 시 종료
		if(current == DestNode) {
			currentPath = MoveTemp(searchedPath);
			ReconstructPath(currentPath,DestNode,FColor::Magenta);
			return;
		}
		
		for (auto neighborNode : current->GetConnectedNode()) {
			#c 가중치 값을 포함한 Cost 값 생성
			double newCost = neighborNode->NodeCost + pathCost[current];
			#d 가중치값을 비교하여 삽입
			if(!searchedPath.Find(neighborNode) || pathCost[neighborNode] > newCost) {
				searchedPath.Add(neighborNode, current);
				pathCost.Add(neighborNode,newCost);
				frontier.Push(neighborNode, newCost);
				ReconstructPath(searchedPath,neighborNode);
			}
		}
	}
}

```

우선순위 큐를 사용하여 구현할 수 있으며, 앞서 모든 경로를 확인하는 BFS와 달리 가중치 조건에 따라 탐색하므로 효율적이며 다용도 목적 길찾기를 적용할 수 있다.

다만 다익스트라의 단점으로 음수 가중치 그래프는 작동하지 않는다는 점, 가중치 값을 추가로 저장하기에 메모리를 추가로 필요로한다는 것이다.

![image](https://github.com/user-attachments/assets/e1508528-e36c-44b6-b8db-ea114a3782b0)
> 마지막에 표기된 숫자는 현재 노드가 가진 가중치의 값이다. 음수 가중치가 없는 조건에서 다익스트라는 누적 가중치가 가장 낮은 경로를 찾는다.
![image](https://github.com/user-attachments/assets/52a75b43-5225-4c60-8fb3-ea4841e29767)
>> 최적화 가중치 경로를 제공



![Dijkstra 프로파일링](https://github.com/user-attachments/assets/7a5b9ab2-0a78-4ffe-b0f0-defd5fcc8236)
> BFS에 가중치가 포함된 개념으로 모든 경로를 탐색하진 않기에 BFS보다 효율적이며 가중치를 기반으로 한 다양한 조건 길찾기 수행이 가능하다.

BFS와 달리 Dijkstra는 가중치를 반영하며, 음수 가중치가 없으면 누적 비용이 가장 낮은 경로를 찾는다. A*는 휴리스틱으로 목표 방향을 우선해 탐색 노드 수를 줄일 수 있다.

---

### A*
![image](https://github.com/user-attachments/assets/2957d6c8-bfe3-4fd2-bf97-74d7ea26b466)

&nbsp;&nbsp;&nbsp; *Astar*는 다익스트라 마찬가지로 우선순위 큐를 사용하며 경로 추정을 위해 `휴리스틱` 을 사용한다. `휴리스틱`은 경로 탐색을 위해 사용하는 계산 함수로 해당 함수의 성능이 탐색 성능에 크게 영향을 미치는것이 A*의 특징이다.


### 휴리스틱
휴리스틱은 두 지점간의 거리를 구해 길찾기 성능을 올릴 때 사용하는 계산 함수이다. A*는 휴리스틱에 큰 의존도를 가진다.

대표적인 휴리스틱은 `유클리드 거리`와 `맨허튼 거리` 가 있다.
![Heuristic](https://github.com/user-attachments/assets/566b5e2f-3601-4dff-9f9c-e4178ce43b3f)
구현할 때 A*에 보편적으로 사용되는 (직선거리 측정법) 유클리드 거리를 적용하였다. 

<br>

A*의 작동순서는 아래와 같다.
1. 시작 노드를 frontier에 넣는다
2. 인접한 노드들에 탐색 여부, 가중치와 현재 가중치를 비교하여 탐색한다.
	2-2. 휴리스틱 계산을 통해 우선순위 큐를 재정렬한다.
3. 도착지점 도달시 유효 경로만 포함하여 출력한다.

```cpp
#A*

void APFinder::A_star_Algorithm() {
	// property.....
	while (!frontier.IsEmpty()) {
		APFNode* current = frontier.Pop();
		#a 도착 조건 설정
		if(current == DestNode) {
			currentPath = MoveTemp(searchedPath);
			ReconstructPath(currentPath,DestNode,FColor::Yellow);
			return;
		}
		for (auto neighborNode : current->GetConnectedNode()) {
			double newCost = neighborNode->NodeCost + pathCost[current];
			if(!searchedPath.Find(neighborNode) || pathCost[neighborNode] > newCost) {
				searchedPath.Add(neighborNode, current);
				pathCost.Add(neighborNode,newCost);
				#b 가중치와 거리를 이용한 우선순위 값 설정 
				double priority = newCost + Heuristic(neighborNode, DestNode);
				frontier.Push(neighborNode, priority);
				ReconstructPath(searchedPath, neighborNode);
			}
		}
	}
}
```

A*는 휴리스틱 함수를 사용함에따라 최단거리 길찾기를 빠르게 수행가능하나, 이는 곧 휴리스틱 함수 의존도가 크다는것을 의미한다. A*의 사용 조건에 대응하는 적절한 휴리스틱을 설정하는것이 핵심이다.

![image](https://github.com/user-attachments/assets/413a8dca-a232-4786-bc49-ebd0e41d156e)
> A*가 최단 경로를 보장하려면 휴리스틱이 실제 남은 비용을 과대평가하지 않아야 한다. 이 구현은 유클리드 거리를 사용하고 이동 비용은 노드의 `NodeCost`를 더하므로, 두 값의 단위가 맞지 않으면 최단 경로를 보장하지 않는다.

![Astar](https://github.com/user-attachments/assets/08c0ab82-3e9b-43c1-a700-255ae07fa7c5)
> 휴리스틱을 이용해 최단거리를 탐색하므로, 이에 해당하지 않는 노드들은 탐색하지 않게 된다. 
> 
> 탐색 경로 저장 메모리는 화면에 나타난 프로파일링 결과를 참고한다.

A*는 추정잔여거리를 사용하기에 거리가 가까울수록 성능이 낮아지는 경우가 존재한다. 이러한 경우를 대비한 JPS를 사용하기도 한다. 길찾기에 보편적으로 A*를 사용하나 지형이 변경되는 경우가 많다면 유연한 대응이 어려울 것이다.

---

## 프로젝트 결과

### 결과 : 
길찾기 알고리즘을 구현하였으며, 각 알고리즘을 적용해보았다. 부족한 부분을 보완하며 효율적인 길찾기 방법을 배울 수 있었다.

### 배운 점 : 
- 길찾기에 사용되는 알고리즘과 각각의 차이점을 알게되었다.
	- Dijkstra의 간선 가중치 , 음수 가중치를 위한 벨만 - 포드 알고리즘에 대한 이해
	- BFS, A* 와 Dijkstra의 차이점 (휴리스틱 사용으로 길찾기 최적화)

-  길찾기 성능
	- BFS는 최단경로를 보장하지만 속도, 메모리 사용량 효율이 떨어진다
	- Dijkstra는 음수 간선이 없으면 최단경로를 보장하며, 가중치를 활용한 상황 시뮬레이션에 적합하고, 가중치 조건으로 BFS에 비해 효율적 탐색 수행
	- A*는 휴리스틱을 이용한 최단거리 계산을 통해 연산 속도, 메모리 사용량에 이점이 있으나, 거리 계산이 포함되므로 유연한 사용에 어려움이 있다.
- 성능 프로파일링 경험
	-  (시간, 메모리 사용)에 대해 깊게 공부해볼 수 있었다. 프로파일링을 통해 메모리 사용량을 확인할 수 있었으며, 각 알고리즘마다의 장단점을 확인할 수 있었다.
 
### 개선 방안 : 
- 우선순위 큐 제작에 사용한 CustomPriorityQueue는 내부에 Array.heapify로 구현된 이진트리를 사용하는데, priority queue를 직접적으로 구현한다면 더욱 효율적인 우선순위 정렬이 가능할것이라 판단함.
- 패스 탐색 정보를 `TMap<APFNode*, APFNode*> searchedPath;`사용하는데 요소 수가 적은 경우 TMap보다 더 효율적인 TSortedMap가 있다. std::map과 유사하지만 레드블랙 트리로 구현되지 않고 배열로 구현되므로 추가, 제거에 선형 시간이 걸려 유리하다.
- SearchedPath와 PathCost를 TMap 대신 TArray로 변경하고, 노드 인덱스를 키로 사용하면 메모리 사용량과 접근 속도를 향상 시킬 수 있을것이다.

- 언리얼의 `Navmesh` 기능에 타일 단위로 관리하여 메모리 사용과 성능을 최적화하고있다.  이처럼 레벨내 월드를 파티셔닝하여 부분적 관리한다면 성능을 최적화 할 수 있을것이다.

### 개인적 소감:

길찾기는 게임에서 중요한 기능이며, RPG의 AI가 플레이어에게 주는 경험에도 영향을 준다고 생각한다. 이번 구현에서 기능 구현에 집중하며 지형 변화 대응 등 세부적인 한계를 확인했다. 알고리즘 비교와 프로파일링 경험을 바탕으로 더 현실감 있는 RPG를 만드는 데 적용하고 싶다.
