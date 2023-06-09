#include "Civilian.h"

ACivilian::ACivilian()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACivilian::BeginPlay()
{
	Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("BEGIN CIVILIAN"));

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANavNode::StaticClass(), AllNodes);
	
	StartNode = GetRandomNode();
    StartNode->EndTarget();
    SetActorLocation(StartNode->GetActorLocation());
	Path.Add(StartNode);
}

// Called every frame
void ACivilian::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Act based on current State
    switch(state)
    {
    case CivilianStates::Pathing:
        
        // Move Towards Target
        Direction = (Path[0]->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        NewLocation = GetActorLocation() + (Direction * Speed * DeltaTime);
        SetActorLocation(NewLocation);

        if (FVector::Distance(GetActorLocation(), Path[0]->GetActorLocation()) < distanceToContact)
        {
            if (Path.Num() == 1)
            {
                SetIdle();
                state = CivilianStates::Idle;
            }
            else
            {
                Path.RemoveAt(0);
            }
        }

        break;

    case CivilianStates::Calculating:
        StartNode = Path[0];
        if(TargetNode) TargetNode->EndTarget();
        TargetNode = GetRandomNode();

        Calculate();

        state = CivilianStates::Pathing;
        break;

    case CivilianStates::Idle:
        IdleCurrentTime -= DeltaTime;
        if (IdleCurrentTime < 0.0)
        {
            state = CivilianStates::Calculating;
        }
        break;
    }
}

void ACivilian::Calculate()
{
    UE_LOG(LogTemp, Log, TEXT("CALCULATING ROUTE"));
    // Reset Everything
    TouchedNodes.Empty();
    VisitedNodes.Empty();
    Path.Empty();

    FNavNodeInfo StartNodeInfo(StartNode, 0.0f, CalculateHeuristic(StartNode), nullptr);

    // Add starting node to touched
    TouchedNodes.Add(StartNodeInfo);

    // A* Algorithm
    while (TouchedNodes.Num() > 0)
    {
        // Find the lowest cost touched node
        FNavNodeInfo CurrentNodeInfo = TouchedNodes[0];
        int32 LowestCostIndex = 0;
        for (int32 i = 1; i < TouchedNodes.Num(); ++i)
        {
            if (TouchedNodes[i]._Cost + TouchedNodes[i]._Heuristic < CurrentNodeInfo._Cost + CurrentNodeInfo._Heuristic)
            {
                CurrentNodeInfo = TouchedNodes[i];
                LowestCostIndex = i;
            }
        }

        TouchedNodes.RemoveAt(LowestCostIndex);
        VisitedNodes.Add(CurrentNodeInfo);

        // Check if the current node is the target node
        if (CurrentNodeInfo._NavNode == TargetNode)
        {
            Path.Empty();
            Path.Add(TargetNode);
            FNavNodeInfo* CurrentNodeInfoPointer = CurrentNodeInfo._ParentNode;
            while (CurrentNodeInfoPointer && CurrentNodeInfoPointer->_ParentNode)
            {
                Path.Insert(CurrentNodeInfoPointer->_NavNode, 0);
                CurrentNodeInfoPointer = CurrentNodeInfoPointer->_ParentNode;
            }

            // Clean up TouchedNodes
            for (FNavNodeInfo& NodeInfo : TouchedNodes)
            {
                delete NodeInfo._ParentNode;
            }

            // Clean up VisitedNodes
            for (FNavNodeInfo& NodeInfo : VisitedNodes)
            {
                delete NodeInfo._ParentNode;
            }

            return;
        }

        // Check Neighbors
        TArray<ANavNode*> Neighbors = CurrentNodeInfo._NavNode->GetConnectedNodes();
        for (ANavNode* Neighbor : Neighbors)
        {
            if (!Neighbor)
            {
                continue;
            }

            // Check if already Visited
            if (VisitedNodes.ContainsByPredicate([&](const FNavNodeInfo& Info) { return Info._NavNode == Neighbor; }))
            {
                continue;
            }

            // Calculate the cost to the neighbor
            float NeighborCost = CurrentNodeInfo._Cost + CalculateDistance(CurrentNodeInfo._NavNode, Neighbor);

            // Check if it has already been Touched
            int32 NeighborIndex = TouchedNodes.IndexOfByPredicate([&](const FNavNodeInfo& Info) { return Info._NavNode == Neighbor; });

            if (NeighborIndex != INDEX_NONE)
            {
                // Neighbor node already touched
                FNavNodeInfo& NeighborInfo = TouchedNodes[NeighborIndex];

                // Check if the new cost is lower
                if (NeighborCost < NeighborInfo._Cost)
                {
                    NeighborInfo._Cost = NeighborCost;
                    delete NeighborInfo._ParentNode; // Delete the previous _ParentNode
                    NeighborInfo._ParentNode = new FNavNodeInfo(CurrentNodeInfo._NavNode, CurrentNodeInfo._Cost, CurrentNodeInfo._Heuristic, CurrentNodeInfo._ParentNode);
                }
            }
            else
            {
                // Add to touched
                FNavNodeInfo NeighborInfo(Neighbor, NeighborCost, CalculateHeuristic(Neighbor), new FNavNodeInfo(CurrentNodeInfo._NavNode, CurrentNodeInfo._Cost, CurrentNodeInfo._Heuristic, CurrentNodeInfo._ParentNode));
                TouchedNodes.Add(NeighborInfo);
            }
        }
    }
}

float ACivilian::CalculateHeuristic(ANavNode* Node)
{
    return CalculateDistance(Node, TargetNode);
}

float ACivilian::CalculateDistance(ANavNode* NodeA, ANavNode* NodeB)
{
    return FVector::Distance(NodeA->GetActorLocation(), NodeB->GetActorLocation());
}

ANavNode* ACivilian::GetRandomNode() const
{
    if (AllNodes.Num() == 0)
    {
        return nullptr;
    }

    while (true)
    {
        int32 RandomIndex = FMath::RandRange(0, AllNodes.Num() - 1);
        ANavNode* NewTarget = Cast<ANavNode>(AllNodes[RandomIndex]);
        if (NewTarget->StartTarget())
        {
            return NewTarget;
        }
    }
}

void ACivilian::SetIdle()
{
    IdleCurrentTime = FMath::FRandRange(IdleMinTime, IdleMaxTime);
}
