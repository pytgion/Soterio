// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "GameTypes.h"


#include "QuestManager.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOTERIO_API AQuestManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	AQuestManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* QuestData;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests", meta = (AllowPrivateAccess = "true"))
	TArray<FQuest> Quests;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true"))
	bool bDebugScreen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true"))
	bool bDebugConsole = false;

protected:
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void InitData();

	void PushQuest(int Id);

	void SetupConsoleCommands();
};