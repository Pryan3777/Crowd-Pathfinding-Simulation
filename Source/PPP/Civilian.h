// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include <queue>

#include "NavNode.h"
#include "NavNodeInfo.h"
#include "DrawDebugHelpers.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimBlueprint.h"
#include "Avoid.h"

#include "Civilian.generated.h"

enum class CivilianStates : uint8
{
	Idle,
	Pathing,
	Calculating,
	Smoothing,
	SmoothingPrep
};

UCLASS()
class PPP_API ACivilian : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACivilian();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		float Speed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline")
		USplineComponent* SplineComponent;

private:
	USkeletalMesh* SkeletalMesh;
	USkeletalMeshComponent* SkeletalMeshComponent;
	UAnimBlueprint* AnimationBlueprint;
	UClass* AnimationClass;
	UCapsuleComponent* CapsuleComponent;
	UCharacterMovementComponent* MovementComponent;
	USceneComponent* SceneComponent;
	UAvoid* AvoidComponent;

	TArray<FNavNodeInfo> TouchedNodes;
	TArray<FNavNodeInfo> VisitedNodes;
	TArray<ANavNode*> Path;
	ANavNode* StartNode;
	ANavNode* TargetNode;

	TArray<AActor*> AllNodes;

	double distanceToContact = 100.0;
	double MoveSpeed = 200.0;
	double IdleMaxTime = 5.0;
	double IdleMinTime = 1.0;
	double IdleCurrentTime = 10.0;
	double SmoothingProgress = 0.0;
	double SmoothingMultiplier = 1.0f;
	double RotationSpeed = 4.0;
	double ToSplinePriority = 0.2f;
	double HeightOffGround = 110.0f;
	double MeshOffset = -90.0f;

	FVector RPoint1;
	FVector RPoint2;
	FVector Center;

	FVector RecentCenter;

	bool FirstNode = true;

	double Direction = 0.0f;
	FVector DirectionVector;
	FVector NewLocation;
	float DistanceAlongSpline;
	FVector SplinePoint;
	FVector SplineTangent;
	FVector ToSpline;

	FVector DesiredDirectionVector;
	double DesiredDirection;
	double DeltaDirection;
	double theta;

	TArray<AActor*> Avoid;

	CivilianStates state = CivilianStates::Calculating;

	void Calculate();

	float CalculateHeuristic(ANavNode* Node);
	float CalculateDistance(ANavNode* NodeA, ANavNode* NodeB);

	ANavNode* GetRandomNode() const;

	void SetIdle();

	FVector LERP(FVector A, FVector B, double progress);
	FVector RLERP(FVector A, FVector B, double progress);
	double GetArcLength(const FVector& StartVector, const FVector& EndVector, double Radius);

	void DrawPath();
	void DrawDebugSpline();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetAvoid(TArray<AActor*> avoid);
};

struct CompareNavNodeInfo
{
	bool operator()(const FNavNodeInfo& A, const FNavNodeInfo& B) const
	{
		return A._Cost + A._Heuristic > B._Cost + B._Heuristic;
	}
};
