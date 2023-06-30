// Fill out your copyright notice in the Description page of Project Settings.


#include "Avoid.h"
#include "PPPGameInstance.h"

// Sets default values for this component's properties
UAvoid::UAvoid()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAvoid::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (GameInstance)
	{
		UPPPGameInstance* PPPGameInstance = Cast<UPPPGameInstance>(GameInstance);
		if (PPPGameInstance)
		{
			PPPGameInstance->AddAvoidReference(GetOwner());
		}
	}
}


// Called every frame
void UAvoid::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

