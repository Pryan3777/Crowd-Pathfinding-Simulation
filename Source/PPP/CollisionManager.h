// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CollisionManager.generated.h"

UCLASS()
class PPP_API ACollisionManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACollisionManager();

	static ACollisionManager* GetInstance();

	void AddActorReference(AActor* ActorToAdd);
	void RemoveActorReference(AActor* ActorToRemove);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	static ACollisionManager* Instance;

	TArray<AActor*> ActorReferences;
};
