// Fill out your copyright notice in the Description page of Project Settings.

#include "QuestManager.h"

AQuestManager::AQuestManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AQuestManager::BeginPlay()
{
	Super::BeginPlay();
	InitData();
}

void AQuestManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ...
}

void AQuestManager::InitData()
{
	if (QuestData)
	{
		static const FString ContextString(TEXT("Quest Context"));

		// Get all rows of the FQuestData structure
		TArray<FQuest*> AllRows;
		QuestData->GetAllRows<FQuest>(ContextString, AllRows);

		int8 i = 0;
		for (FQuest* QuestRow : AllRows)
		{
			if (QuestRow)
			{
				if (bDebugConsole)
				{
				UE_LOG(LogTemp, Warning, TEXT("Name: %s"), *QuestRow->Name.ToString());
				UE_LOG(LogTemp, Warning, TEXT("Description: %s"), *QuestRow->Description.ToString());
				UE_LOG(LogTemp, Warning, TEXT("Reward: %f"), QuestRow->Reward);
				}

				QuestRow->Id = i;
				Quests.Add(*QuestRow);
				i++;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("QuestDataTable is not loaded!"));
	}
}

void AQuestManager::PushQuest(int Id)
{
	int i = 0;
	for (FQuest Quest : Quests)
	{
		if (Id == Quest.Id)
		{
			GEngine->AddOnScreenDebugMessage(35, 4.0f, FColor::Magenta, TEXT("Quest Found!"));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(35, 4.0f, FColor::Red, TEXT("Quest NOT Found!"));
		}
		i++;
	}
}

void AQuestManager::SetupConsoleCommands()
{
}