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

    Speed = MoveSpeed;
}

// Called every frame
void ACivilian::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    double ratio;

    // Act based on current State
    switch(state)
    {
    case CivilianStates::Pathing:
        Speed = MoveSpeed;

        // Move Towards Target
        DesiredDirectionVector = (Path[0]->GetActorLocation() - GetActorLocation());
        DesiredDirectionVector.Z = 0.0f;
        DesiredDirectionVector = DesiredDirectionVector.GetSafeNormal();
        DesiredDirection = atan2(DesiredDirectionVector.Y, DesiredDirectionVector.X);
        DesiredDirection = fmod((DesiredDirection), 2 * PI);
        if (DesiredDirection < -PI)
        {
            DesiredDirection += 2.0 * PI;
        }
        else if (DesiredDirection > PI)
        {
            DesiredDirection -= 2.0 * PI;
        }
        DeltaDirection = fmod((DesiredDirection - Direction), 2 * PI);
        if (DeltaDirection < -PI)
        {
            DeltaDirection += 2.0 * PI;
        }
        else if(DeltaDirection > PI)
        {
            DeltaDirection -= 2.0 * PI;
        }
        theta = RotationSpeed * DeltaTime;

        if (FMath::Abs(DeltaDirection) < theta)
        {
            Direction = DesiredDirection;
            ratio;
        }
        else if (FMath::Abs(DeltaDirection) > PI / 2)
        {
            ratio = FMath::Abs(DeltaDirection) / (PI / 2);
            Speed = Speed * (2.0 - ratio);
            Direction += theta * ratio * FMath::Sign(DeltaDirection);
        }
        else
        {
            Direction += theta * FMath::Sign(DeltaDirection);
        }
        
        DirectionVector = FVector(cos(Direction), sin(Direction), 0.0).GetSafeNormal();
        SetActorRotation(DirectionVector.Rotation());
        NewLocation = GetActorLocation() + (DirectionVector * Speed * DeltaTime);
        NewLocation.Z = 0.0;
        SetActorLocation(NewLocation);

        if (FVector::Distance(GetActorLocation(), Path[0]->GetActorLocation()) <= Path[0]->SmoothingRadius)
        {
            if (Path.Num() == 1)
            {
                SetIdle();
                state = CivilianStates::Idle;
            }
            else
            {
                //state = CivilianStates::SmoothingPrep;

                Path.RemoveAt(0);
                state = CivilianStates::Pathing;
            }
        }

        break;

    case CivilianStates::Calculating:
        Speed = 0.0f;

        StartNode = Path[0];
        if(TargetNode) TargetNode->EndTarget();
        TargetNode = GetRandomNode();

        Calculate();

        state = CivilianStates::Pathing;
        break;

    case CivilianStates::Idle:
        Speed = 0.0f;

        IdleCurrentTime -= DeltaTime;
        if (IdleCurrentTime < 0.0)
        {
            state = CivilianStates::Calculating;
        }
        break;

    /*case CivilianStates::SmoothingPrep:
        Center = Path[0]->GetActorLocation();
        RPoint1 = Center + (RecentCenter - Center).GetSafeNormal() * Path[0]->SmoothingRadius;
        RPoint2 = Center + (Path[1]->GetActorLocation() - Center).GetSafeNormal() * Path[0]->SmoothingRadius;
        SmoothingProgress = 0.0f;
        SmoothingMultiplier = GetArcLength(RPoint1 - Center, RPoint2 - Center, Path[0]->SmoothingRadius);

        state = CivilianStates::Smoothing;
        break;*/

    /*case CivilianStates::Smoothing:
        if (SmoothingProgress >= 1.0f)
        {
            RecentCenter = Path[0]->GetActorLocation();
            Path.RemoveAt(0);
            state = CivilianStates::Pathing;
        }
        SmoothingProgress += Speed * DeltaTime / (SmoothingMultiplier);

        Direction = RLERP(Center - RPoint1, RPoint2 - Center, SmoothingProgress);
        SetActorRotation(Direction.Rotation());
        NewLocation = LERP(LERP(RPoint1, Center, SmoothingProgress), LERP(Center, RPoint2, SmoothingProgress), SmoothingProgress);
        SetActorLocation(NewLocation);

        break;*/
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
            if (CurrentNodeInfoPointer)
            {
                RecentCenter = CurrentNodeInfoPointer->_NavNode->GetActorLocation();
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

FVector ACivilian::LERP(FVector A, FVector B, double progress)
{
    if (progress < 0.0f)
    {
        return A;
    }
    if (progress > 1.0f)
    {
        return B;
    }
    return (A * (1.0 - progress)) + (B * progress);
}

FVector ACivilian::RLERP(FVector A, FVector B, double progress)
{
    // Normalize the input vectors
    FVector DirectionA = A.GetSafeNormal();
    FVector DirectionB = B.GetSafeNormal();

    // Interpolate between the directions
    FVector InterpolatedDirection = DirectionA + progress * (DirectionB - DirectionA);

    // Normalize the interpolated direction
    InterpolatedDirection.Normalize();

    return InterpolatedDirection;
}

double ACivilian::GetArcLength(const FVector& StartVector, const FVector& EndVector, double Radius)
{
    // Calculate the angle between the start and end vectors
    double Angle = FVector::DotProduct(StartVector.GetSafeNormal(), EndVector.GetSafeNormal());

    Angle = FMath::Acos(Angle);

    const double Pi = 3.14159265;

    double top = Radius * (Pi - Angle) * FMath::Sin(Angle / 2.0);
    double bottom = FMath::Sin((Pi - Angle) / 2.0);

    return top/bottom;
}