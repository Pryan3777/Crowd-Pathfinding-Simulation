// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Avoid.h"
#include "Civilian.h"
#include "PPPGameInstance.generated.h"

/**
 * 
 */

class ACivilian;

UCLASS()
class PPP_API UPPPGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	void AddAvoidReference(AActor* ActorToAdd);
	void RemoveAvoidReference(AActor* ActorToRemove);

	void AddCivilianReference(ACivilian* CivilianToAdd);
	void RemoveCivilianReference(ACivilian* CivilianToRemove);
private:
	void UpdateReferences();

	TArray<ACivilian*> CivilianReferences;
	TArray<AActor*> AvoidReferences;
};
