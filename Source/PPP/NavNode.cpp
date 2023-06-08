// Fill out your copyright notice in the Description page of Project Settings.


#include "NavNode.h"


// Sets default values
ANavNode::ANavNode()
{
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent->SetMobility(EComponentMobility::Movable);

    SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
    SplineComponent->SetMobility(EComponentMobility::Movable);
    SplineComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

    // Create and attach the node mesh component
    NodeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NodeMeshComponent"));
    NodeMeshComponent->SetupAttachment(RootComponent);
    NodeMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Set the static mesh for the node
    ConstructorHelpers::FObjectFinder<UStaticMesh> NodeMeshAsset(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));
    if (NodeMeshAsset.Succeeded())
    {
        UStaticMesh* LoadedMesh = NodeMeshAsset.Object;
        if (LoadedMesh)
        {
            UE_LOG(LogTemp, Warning, TEXT("Mesh asset loaded successfully."));
            NodeMeshComponent->SetStaticMesh(LoadedMesh);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load mesh asset."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find mesh asset."));
    }

    RootComponent->SetHiddenInGame(true);
}

void ANavNode::ConditionalBeginDestroy()
{
    ConnectedNodes.Empty();
    UpdateVisualConnections();

    Super::ConditionalBeginDestroy();
}

void ANavNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
     // Update spline component for visual connections when any property changes
     UpdateVisualConnections();
}

// Called when the game starts or when spawned
void ANavNode::BeginPlay()
{
    AActor::BeginPlay();
}

// Called every frame
void ANavNode::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);

}

void ANavNode::PostEditMove(bool bFinished)
{
    AActor::PostEditMove(bFinished);

    // Update spline component for visual connections
    UpdateVisualConnections();
}

void ANavNode::UpdateVisualConnections()
{
    bool changed = false;
    
    ConnectedNodes.Remove(this);
    int32 nulls = ConnectedNodes.Remove(nullptr);

    ConnectedNodes.StableSort();

    if (nulls > 0)
    {
        ConnectedNodes.Add(nullptr);
    }

    ANavNode* recent = nullptr;
    for (int i = 0; i < ConnectedNodes.Num(); i++)
    {
        if (ConnectedNodes[i] == recent && i != 0)
        {
            ConnectedNodes.RemoveAt(i);
            i--;
        }
        else
        {
            recent = ConnectedNodes[i];
        }
    }

    for (ANavNode* ConnectedNode : ConnectedNodes)
    {
        if (ConnectedNode && !ConnectedNodesPast.Contains(ConnectedNode))
        {
            ConnectedNode->AddConnection(this);
            changed = true;
        }
    }
    for (ANavNode* ConnectedNodePast : ConnectedNodesPast)
    {
        if (ConnectedNodePast && !ConnectedNodes.Contains(ConnectedNodePast))
        {
            ConnectedNodePast->RemoveConnection(this);
            changed = true;
        }
    }

    if (changed)
    {
        ConnectedNodesPast = ConnectedNodes;
    }
    DrawConnections();
    SplineComponent->UpdateSpline();
}

void ANavNode::AddConnection(ANavNode * target)
{
    if (!target || target == this)
    {
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("AddingConnection"));
    int32 size = ConnectedNodes.Num();
    int32 index = ConnectedNodes.Add(target);
    ConnectedNodesPast.Add(target);
    if (size == index)
    {
        DrawConnections();
    }
}

void ANavNode::RemoveConnection(ANavNode* target)
{
    if (!target || target == this)
    {
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("RemovingConnection"));
    int32 removed = ConnectedNodes.Remove(target);
    ConnectedNodesPast.Remove(target);
    if (removed > 0)
    {
        DrawConnections();
    }
}

void ANavNode::DrawConnections(bool propogate)
{
    float sPosition = 0.0f;
    int nonNullNodeCount = ConnectedNodes.Num() - (ConnectedNodes.Num() > 0 && ConnectedNodes[ConnectedNodes.Num() - 1] == nullptr ? 1 : 0);
    float sIncrement = 1.0f / (float)(nonNullNodeCount * 2);
    if (SplineComponent)
    {
        SplineComponent->ClearSplinePoints();

        for (ANavNode* ConnectedNode : ConnectedNodes)
        {
            if (ConnectedNode)
            {
                FVector StartLocation = GetActorLocation();
                FVector EndLocation = ConnectedNode->GetActorLocation();

                FVector LocalStartLocation = SplineComponent->GetComponentTransform().InverseTransformPosition(StartLocation);
                FVector LocalEndLocation = SplineComponent->GetComponentTransform().InverseTransformPosition(EndLocation);

                FSplinePoint StartPoint(sPosition, LocalStartLocation, ESplinePointType::Constant);
                sPosition += sIncrement;
                FSplinePoint EndPoint(sPosition, LocalEndLocation, ESplinePointType::Constant);
                sPosition += sIncrement;

                SplineComponent->AddPoint(StartPoint, ESplineCoordinateSpace::World);
                SplineComponent->AddPoint(EndPoint, ESplineCoordinateSpace::World);

                if (propogate)
                {
                    ConnectedNode->DrawConnections(false);
                }
            }
        }

        {
            FVector StartLocation = GetActorLocation();
            FVector LocalStartLocation = SplineComponent->GetComponentTransform().InverseTransformPosition(StartLocation);
            FSplinePoint StartPoint(1.0f, LocalStartLocation, ESplinePointType::Constant);
            SplineComponent->AddPoint(StartPoint, ESplineCoordinateSpace::World);
        }
        SplineComponent->UpdateSpline();
    }
}

TArray<ANavNode*>& ANavNode::GetConnectedNodes()
{
    return ConnectedNodes;
}
