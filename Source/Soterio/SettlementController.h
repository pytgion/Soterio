// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "EngineUtils.h"

#include "GameTypes.h"
#include "SoterioCharacter.h"
#include "Settlement.h"

#include "SettlementController.generated.h"

UCLASS()
class SOTERIO_API ASettlementController : public AActor
{
	GENERATED_BODY()
private:
	int activeSettlementCount;

	TMap<FString, TMap<FString, float>> DistanceCache;

	TArray<USettlement*> Settlements;

	static ASettlementController* ActiveSettlementController;
public:
	// Sets default values for this actor's properties
	ASettlementController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugScreen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugConsole = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* SettlementData;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;
public:

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settlement Data")
	UDataTable* SettlementDataTable;

	void InitSettlements();

	void SettlementReputationUpdate();

	void CookDistanceData();

	void DebugSettlements();

	void CalculateInflunceRate();

	void SetupConsoleCommands();

	float CalculateRenownForSettlement(USettlement* Settlement);

	void ChangeSettlementReputation(FString SettlementName, float NewReputation);
};