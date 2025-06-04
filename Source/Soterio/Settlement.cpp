// Fill out your copyright notice in the Description page of Project Settings.

#include "Settlement.h"

#include "SoterioGameMode.h"
// Sets default values for this component's properties
USettlement::USettlement()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Name = "Debug Village";
	Description = "An example Village with debug purpose";

	RenownBy = 50.0f;
	ReputationBy = 25.0f;

	ProgressRate = 0.05f;

	bDebug = true;
}

// Called when the game starts
void USettlement::BeginPlay()
{
	Super::BeginPlay();
	CalculateProgressRate();

	// TODO: Implement Time managament
	//AMyProjectGameMode* GameMode = ASoterioGameMode::GetActiveGameMode();
	//if (GameMode)
	//{
	//	GameMode->OnNewDayStartDelegate.Add(FOnNewDayStart::FDelegate::CreateUObject(this, &USettlement::OnNewDay));
	//	if (bDebug)
	//		UE_LOG(LogTemp, Warning, TEXT("Bound USettlement::OnNewDay to the GameMode delegate."));
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("Failed to get GameMode in USettlement::BeginPlay"));
	//}
}

// Called every frame
void USettlement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void USettlement::SetInflunceRate(float Influnce)
{
	this->S_InflunceRate = Influnce;
}

void USettlement::OnNewDay()
{
	UE_LOG(LogTemp, Warning, TEXT("New day started in settlement!"));
	CalculateProgressRate();
	RenownBy += ProgressRate;
}

void USettlement::InitData(FSettlementData* Data)
{
	this->SettlementData = Data;
}