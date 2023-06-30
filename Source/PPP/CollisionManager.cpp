// Fill out your copyright notice in the Description page of Project Settings.


#include "CollisionManager.h"


ACollisionManager* ACollisionManager::Instance = nullptr;

// Sets default values
ACollisionManager::ACollisionManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

ACollisionManager* ACollisionManager::GetInstance()
{
    if (!Instance)
    {
        UWorld* World = GEngine->GetWorld();
        
        Instance = World->SpawnActor<ACollisionManager>(ACollisionManager::StaticClass());
    }

    return Instance;
}

// Called when the game starts or when spawned
void ACollisionManager::BeginPlay()
{
	Super::BeginPlay();

    if (GetInstance() != this)
    {
        Destroy();
    }
	
}

// Called every frame
void ACollisionManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACollisionManager::AddActorReference(AActor* ActorToAdd)
{
    if (ActorToAdd && !ActorReferences.Contains(ActorToAdd))
    {
        ActorReferences.Add(ActorToAdd);
    }
}

void ACollisionManager::RemoveActorReference(AActor* ActorToRemove)
{
    if (ActorToRemove)
    {
        ActorReferences.Remove(ActorToRemove);
    }
}