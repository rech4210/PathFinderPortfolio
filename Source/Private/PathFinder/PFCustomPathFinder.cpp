// Fill out your copyright notice in the Description page of Project Settings.


#include "PathFinder/PFCustomPathFinder.h"
#include "GameFramework/Actor.h"
#include "Containers/Queue.h"



void UPFCustomPathFinder::CustomPathfinder(const AActor* StartActor,const AActor* EndActor, EPathfindingAlgorithm Algorithm)
{
	if (!StartActor || !EndActor)
	{
		return;
	}

	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(StartActor->GetWorld()))
	{
		//world navmesh의 정보를 가지고있음.
		FPathFindingQuery Query;
		Query.StartLocation = StartActor->GetActorLocation();
		Query.EndLocation = EndActor->GetActorLocation();
		Query.NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::ECreateIfEmpty::Create);

		// 기본 경로 정보를 얻기 위해 사용
		FPathFindingResult Result = NavSys->FindPathSync(Query);
		
		if (Result.IsSuccessful())
		{
			// 기본 경로를 기반으로 커스텀 알고리즘 적용
			switch (Algorithm)
			{
			case EPathfindingAlgorithm::Bfs:
				//UPFCustomPathFinder::BFS_Algorithm(StartActor,Query.NavData,StartActor->GetActorLocation(),EndActor->GetActorLocation());
					// 큐를 사용하여 경로 탐색
						break;
			case EPathfindingAlgorithm::Dijkstra:
				//UPFCustomPathFinder::Dijkstra_Algorithm();
					// 우선순위 큐를 사용하여 경로 탐색
						break;
			case EPathfindingAlgorithm::AStar:
				//UPFCustomPathFinder::A_star_Algorithm();
					// 휴리스틱을 사용하여 경로 탐색
						break;
			case EPathfindingAlgorithm::Jsp:
				//UPFCustomPathFinder::JSP_Algorithm();
					// 휴리스틱을 사용하여 경로 탐색
				break;
			default:
				break;
			}
		}
	}
}


//navmesh와 비교하여 어떤 요소로 길찾기를 수행할지 구체적인 내용 작성하자
//길찾기도 물론이지만, 해당하는 길찾기들에 들어가는 미로 (노드, 비용)를 어떻게 구성할지 생각해보자

FVector3f UPFCustomPathFinder::BFS_Algorithm(AActor* target, FVector3f current, FVector3f arrival) {
	return BFS_Algorithm(target, current, arrival);
}

FVector3f UPFCustomPathFinder::Dijkstra_Algorithm(AActor* target, FVector3f current, FVector3f arrival) {
	return Dijkstra_Algorithm(target, current, arrival);
}

FVector3f UPFCustomPathFinder::A_star_Algorithm(AActor* target, FVector3f current, FVector3f arrival) {
	return A_star_Algorithm(target, current, arrival);
}

FVector3f UPFCustomPathFinder::JSP_Algorithm(AActor* target, FVector3f current, FVector3f arrival) {
	return JSP_Algorithm(target, current, arrival);
}
