// Fill out your copyright notice in the Description page of Project Settings.


#include "PPPGameInstance.h"

void UPPPGameInstance::AddAvoidReference(AActor* ActorToAdd)
{
	AvoidReferences.Add(ActorToAdd);
	UpdateReferences();
}

void UPPPGameInstance::RemoveAvoidReference(AActor* ActorToRemove)
{
	AvoidReferences.Remove(ActorToRemove);
	UpdateReferences();
}

void UPPPGameInstance::AddCivilianReference(ACivilian* CivilianToAdd)
{
	CivilianReferences.Add(CivilianToAdd);
}

void UPPPGameInstance::RemoveCivilianReference(ACivilian* CivilianToRemove)
{
	CivilianReferences.Add(CivilianToRemove);
}

void UPPPGameInstance::UpdateReferences()
{
	for (auto civilian : CivilianReferences)
	{
		civilian->SetAvoid(AvoidReferences);
	}
}
