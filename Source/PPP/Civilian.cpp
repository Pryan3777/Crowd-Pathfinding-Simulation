#include "Civilian.h"
#include "PPPGameInstance.h"

ACivilian::ACivilian()
{

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    //RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    SkeletalMeshComponent = FindComponentByClass<USkeletalMeshComponent>();
    SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
    CapsuleComponent = FindComponentByClass<UCapsuleComponent>();
    SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    AvoidComponent = CreateDefaultSubobject<UAvoid>(TEXT("AvoidComponent"));

    //CapsuleComponent->SetupAttachment(RootComponent);
    RootComponent = CapsuleComponent;
    SceneComponent->SetupAttachment(RootComponent);
    SkeletalMeshComponent->SetupAttachment(CapsuleComponent);
    SplineComponent->SetupAttachment(SceneComponent);

    SplineComponent->SetMobility(EComponentMobility::Static);
    
    RootComponent->SetMobility(EComponentMobility::Movable);
    

    SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny"));
    SkeletalMeshComponent->SetSkeletalMeshAsset(SkeletalMesh);

    AnimationBlueprint = LoadObject<UAnimBlueprint>(nullptr, TEXT("/Game/Animations/AI/AP_EnemyAI"), nullptr, LOAD_None, nullptr);
    AnimationClass = AnimationBlueprint->GeneratedClass;
    SkeletalMeshComponent->SetAnimInstanceClass(AnimationClass);

    SkeletalMeshComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    SkeletalMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, MeshOffset));
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

    Speed = MoveSpeed * SpeedMultiplier;

    UGameInstance* GameInstance = GetWorld()->GetGameInstance();
    if (GameInstance)
    {
        UPPPGameInstance* PPPGameInstance = Cast<UPPPGameInstance>(GameInstance);
        if (PPPGameInstance)
        {
            PPPGameInstance->AddCivilianReference(this);
        }
    }
}

// Called every frame
void ACivilian::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Act based on current State
    switch(state)
    {
    case CivilianStates::Pathing:
        Speed = MoveSpeed * SpeedMultiplier;

        // Move Towards Target
        DesiredDirectionVector = (Path[0]->GetActorLocation() - GetActorLocation());
        DesiredDirectionVector.Z = 0.0f;
        DesiredDirectionVector = DesiredDirectionVector.GetSafeNormal();
       /* DesiredDirection = atan2(DesiredDirectionVector.Y, DesiredDirectionVector.X);
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
        }*/
        //FVector Dot = DotProduct(DesiredDirectionVector, DesiredDirectionVector);
        Center = GetActorLocation();
        Center.Z = 0.0f;
        SetActorLocation(Center);

        DistanceAlongSpline = SplineComponent->FindInputKeyClosestToWorldLocation(GetActorLocation());
        SplinePoint = SplineComponent->GetLocationAtSplineInputKey(DistanceAlongSpline, ESplineCoordinateSpace::World);
        SplineTangent = SplineComponent->GetDirectionAtSplineInputKey(DistanceAlongSpline, ESplineCoordinateSpace::World);
        ToSpline = (SplinePoint - (GetActorLocation()));
        ToSpline.Z = 0.0f;

        DrawDebugSphere(GetWorld(), SplinePoint + FVector(0.0f, 0.0f, 250.0f), 25.0f, 16, FColor::Cyan, false, -1.f, 0, 0.f);

        DrawDebugLine(GetWorld(), GetActorLocation() + FVector(0.0f, 0.0f, 150.0f), GetActorLocation() + FVector(0.0f, 0.0f, 150.0f) + (SplineTangent * 100.0f), FColor::Red, false, -1.0f, 0, 3.0f);
        DrawDebugLine(GetWorld(), GetActorLocation() + FVector(0.0f, 0.0f, 150.0f), GetActorLocation() + FVector(0.0f, 0.0f, 150.0f) + (ToSpline * 100.0f), FColor::Blue, false, -1.0f, 0, 3.0f);
        DrawDebugSpline();

        CalculateAvoid();
        CalculatePredictiveAvoid();

        //DirectionVector = ((DirectionVector * PreviousVectorFactor) + (SplineTangent * SplineTangentFactor) + (ToSpline * ToSplinePriority) + AvoidVector).GetSafeNormal();
        DirectionVector = ((DirectionVector * PreviousVectorFactor) + (SplineTangent * SplineTangentFactor) + (ToSpline * ToSplinePriority) + (PredictiveAvoidVector * PredictiveAvoidScaleFactor) + AvoidVector).GetSafeNormal();
        SetActorRotation(DirectionVector.Rotation());

        CalculateCatchUp();

        NewLocation = GetActorLocation() + (DirectionVector * Speed * DeltaTime * SpeedMultiplier);
        GetComponentByClass<UAvoid>()->SetVelocity(DirectionVector * Speed * SpeedMultiplier);
        NewLocation.Z = HeightOffGround;
        SetActorLocation(NewLocation);

        while (Path.Num() > 1)
        {
            Path.RemoveAt(0);
        }

        if (FVector::Distance(GetActorLocation(), Path[0]->GetActorLocation()) <= Path[0]->SmoothingRadius)
        {
            if (Path.Num() == 1)
            {
                SetIdle();
                state = CivilianStates::Idle;
            }
            //else
            //{
            //    //state = CivilianStates::SmoothingPrep;

            //    Path.RemoveAt(0);
            //    state = CivilianStates::Pathing;
            //}
        }

        break;

    case CivilianStates::Calculating:
        Speed = 0.0f;

        StartNode = Path[0];
        if(TargetNode) TargetNode->EndTarget();
        TargetNode = GetRandomNode();

        Calculate();

        DrawPath();

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
            while (CurrentNodeInfoPointer)
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

void ACivilian::DrawPath()
{
    float sPosition = 0.0f;
    int nonNullNodeCount = Path.Num();
    float sIncrement = 1.0f / (float)(nonNullNodeCount);
    if (SplineComponent)
    {
        SplineComponent->ClearSplinePoints();

        for (ANavNode* ConnectedNode : Path)
        {
            if (ConnectedNode)
            {
                FVector StartLocation = ConnectedNode->GetActorLocation() + FVector(0.0f, 0.0f, 0.0f);

                FVector LocalStartLocation = SplineComponent->GetComponentTransform().InverseTransformPosition(StartLocation);

                FSplinePoint StartPoint(sPosition, LocalStartLocation, ESplinePointType::Curve);
                sPosition += sIncrement;

                SplineComponent->AddPoint(StartPoint, ESplineCoordinateSpace::World);
            }
        }

        SplineComponent->UpdateSpline();
    }
}

void ACivilian::DrawDebugSpline()
{
    int points = 100;
    for (int i = 0; i < points; i++)
    {
        DrawDebugLine(GetWorld(), SplineComponent->GetLocationAtSplineInputKey(((float)i) / (points + 1), ESplineCoordinateSpace::World) + FVector(0.0f, 0.0f, 250.0f), SplineComponent->GetLocationAtSplineInputKey(((float)(i + 1)) / (points + 1), ESplineCoordinateSpace::World) + FVector(0.0f, 0.0f, 250.0f), FColor::Green, false, -1.0f, 0, 3.0f);
    }
}

void ACivilian::SetAvoid(TArray<AActor*> avoid)
{
    Avoid = avoid;
}

void ACivilian::CalculateAvoid()
{
    AvoidVector = FVector(0.0f, 0.0f, 0.0f);

    FVector myLocation = GetActorLocation();
    FVector otherLocation;
    FVector otherToMy;
    double distance;
    int avoidCount = 0;

    for (auto avoid : Avoid)
    {
        otherLocation = avoid->GetActorLocation();
        otherToMy = myLocation - otherLocation;
        distance = (otherToMy).Length();
        otherToMy.Normalize();
        if (avoid != this && GetActorForwardVector().Dot(-otherToMy) > 0.0)
        {
            if (distance < MaxAvoidRange)
            {
                avoidCount++;
                DrawDebugLine(GetWorld(), myLocation + FVector(0.0f, 0.0f, 250.0f), otherLocation + FVector(0.0f, 0.0f, 250.0f), FColor::Purple, false, -1.0f, 0, 10.0f);
                
                AvoidVector += (GetActorForwardVector().Dot(-avoid->GetActorForwardVector()) + 2.0) * GetActorForwardVector().Dot(-otherToMy) * otherToMy * pow(((MaxAvoidRange - distance) / MaxAvoidRange), 3.0 );
            }
        }
    }
    AvoidVector.Z = 0.0f;
    if (avoidCount > 0)
    {
        AvoidVector /= avoidCount;
    }
    AvoidVector *= AvoidScaleFactor;
    
}

void ACivilian::CalculatePredictiveAvoid()
{
    PredictiveAvoidVector = FVector(0.0f, 0.0f, 0.0f);

    FVector SelfPosition = GetActorLocation();
    FVector SelfVelocity = GetComponentByClass<UAvoid>()->GetVelocity();
    for (auto avoid : Avoid)
    {
        if (avoid != this)
        {
            FVector OtherPosition = avoid->GetActorLocation();
            FVector OtherVelocity = avoid->GetComponentByClass<UAvoid>()->GetVelocity();
            double Uax = SelfPosition.X;
            double Ubx = OtherPosition.X;
            double Uay = SelfPosition.Y;
            double Uby = OtherPosition.Y;
            double Vax = SelfVelocity.X;
            double Vbx = OtherVelocity.X;
            double Vay = SelfVelocity.Y;
            double Vby = OtherVelocity.Y;
            double t = -(((Uax - Ubx) * (Vax - Vbx)) + ((Uay - Uby) * (Vay - Vby))) / (((Vax - Vbx) * (Vax - Vbx)) + ((Vay - Vby) * (Vay - Vby)));
            if (t <= 0)
            {
                break;
            }
            DrawDebugLine(GetWorld(), SelfPosition + FVector(0,0,25.0), SelfPosition + (SelfVelocity * t) + FVector(0, 0, 25.0), FColor::Orange, false, -1.0f, 0, 3.0f);
            DrawDebugLine(GetWorld(), OtherPosition + FVector(0, 0, 25.0), OtherPosition + (OtherVelocity * t) + FVector(0, 0, 25.0), FColor::Orange, false, -1.0f, 0, 3.0f);
            FVector CollisionVector = (SelfPosition + (t * SelfVelocity)) - (OtherPosition + (t * OtherVelocity));
            double CollisionVectorMagnitude = CollisionVector.Length();
            CollisionVector.Normalize();
            PredictiveAvoidVector +=  CollisionVector / (pow(t, 0.5) * pow(CollisionVectorMagnitude, 2.0));
        }
    }

    PredictiveAvoidVector.Z = 0.0;
}

void ACivilian::CalculateCatchUp()
{
    SpeedMultiplier = 1.0;
    bool crowd = false;
    double ClosestFollowDistance = 1000000;
    for (auto avoid : Avoid)
    {
        FVector ToAvoid = avoid->GetActorLocation() - GetActorLocation();
        FVector ToAvoidNorm = ToAvoid;
        ToAvoidNorm.Normalize();
        if (ToAvoid.Length() < 500.0 && ToAvoidNorm.Dot(DirectionVector) > 0.85) 
        {
            crowd = true;
            ClosestFollowDistance = FMath::Min(ClosestFollowDistance, ToAvoid.Length());
        }
    }
    if (crowd && ClosestFollowDistance > 200.0)
    {
        SpeedMultiplier = 2.0;
    }
}
