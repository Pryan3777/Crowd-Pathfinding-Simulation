// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include <queue>

#include "NavNode.h"
#include "NavNodeInfo.h"

#include "Civilian.generated.h"

UCLASS()
class PPP_API ACivilian : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACivilian();

private:
	TArray<FNavNodeInfo> TouchedNodes;
	TArray<FNavNodeInfo> VisitedNodes;
	TArray<ANavNode*> Path;
	ANavNode* StartNode;
	ANavNode* TargetNode;

	TArray<AActor*> AllNodes;

	double distanceToContact = 100.0;
	double Speed = 150.0;

	void Calculate();

	float CalculateHeuristic(ANavNode* Node);
	float CalculateDistance(ANavNode* NodeA, ANavNode* NodeB);

	ANavNode* GetRandomNode() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

struct CompareNavNodeInfo
{
	bool operator()(const FNavNodeInfo& A, const FNavNodeInfo& B) const
	{
		return A._Cost + A._Heuristic > B._Cost + B._Heuristic;
	}
};
