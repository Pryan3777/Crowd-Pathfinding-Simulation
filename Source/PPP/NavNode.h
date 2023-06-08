// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"

#include "NavNode.generated.h"

UCLASS()
class PPP_API ANavNode : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANavNode();

	void ConditionalBeginDestroy();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
		FVector Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
		TArray<ANavNode*> ConnectedNodes;

	TArray<ANavNode*> ConnectedNodesPast;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Node", meta = (ShowOnlyInnerProperties))
		USplineComponent* SplineComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Node")
		UStaticMeshComponent* NodeMeshComponent;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void PostEditMove(bool bFinished) override;
	void UpdateVisualConnections();

	void AddConnection(ANavNode* target = nullptr);
	void RemoveConnection(ANavNode* target = nullptr);

	void DrawConnections(bool propogate = true);

	TArray<ANavNode*>& GetConnectedNodes();
};
