// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Settlement.generated.h"

class AMyProjectGameMode;

#define MAX_SETTLEMENT_DISTANCE 5

UENUM()
enum ESettlementType
{
	VILLAGE UMETA(DisplayName = "Village"),
	CITY	UMETA(DisplayName = "City")
};

USTRUCT(BlueprintType)
struct FResourceDuration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Coal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Copper;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Iron;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Gold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Yada;
};

USTRUCT(BlueprintType)
struct FSettlementData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ESettlementType> SettlementType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PriceMultipleBy = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FResourceDuration ResourceDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "8"))
	int Statue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, float> DistanceTo;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOTERIO_API USettlement : public UActorComponent
{
	GENERATED_BODY()

private:
	FVector2D Location;

	float S_InflunceRate;

	FString Name;
	FString Description;

	FSettlementData* SettlementData;

	int screenDebug;

	float RenownBy;
	float ReputationBy;

	float ProgressRate;

public:
	// Sets default values for this component's properties
	USettlement();

	bool bDebug = false;
	TMap<FString, float> DistanceMap;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void setScreenDebug(int val) { this->screenDebug = val; }

	void SetName(FString newName) { this->Name = newName; };

	FString GetName() { return this->Name; };

	void SetDescription(FString newDescription) { this->Description = newDescription; };

	FString GetDescription() { return this->Description; };

	void SetRenownBy(float val) { this->RenownBy = val; };

	float GetRenownBy() { return this->RenownBy; };

	float GetReputation() { return this->ReputationBy; };

	void SetReputation(float val) { this->ReputationBy = val; };

	void AddReputation(float val) { this->ReputationBy += val; };

	void Progress() { this->RenownBy += ProgressRate; };

	//void SetSpeedMultipler(float speed)
	//{
	//	this->speedMultipler = speed;
	//}

	void CalculateProgressRate()
	{
		ProgressRate *= 1 + (ReputationBy / 1000);
	}

	void SetInflunceRate(float Influnce);

	FVector2D* GetLocation() { return &Location; };

	void SetLocation(FVector2D Loc) { this->Location = Loc; };

	void OnNewDay();

	void InitData(FSettlementData* Data);
};