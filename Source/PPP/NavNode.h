// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/Actor.h"

#include "NavNode.generated.h"

UCLASS()
class PPP_API ANavNode : public AActor
{
	GENERATED_BODY()

public:
	ANavNode();

	void ConditionalBeginDestroy();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
		FVector Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
		TArray<ANavNode*> ConnectedNodes;

	TArray<ANavNode*> ConnectedNodesPast;

	USplineComponent* SplineComponent;

	UStaticMeshComponent* NodeMeshComponent;

	UMaterialInstanceDynamic* NodeMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Node")
	int32 TargetCapacity = 0;

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

	int32 TargetCount = 0;

	bool StartTarget();
	void EndTarget();

private:
	void UpdateColor();
};
