// Fill out your copyright notice in the Description page of Project Settings.

#include "SettlementController.h"

#include "SoterioGameMode.h"

ASettlementController* ASettlementController::ActiveSettlementController = nullptr;

// Sets default values
ASettlementController::ASettlementController()
{
	PrimaryActorTick.bCanEverTick = true;

	ASettlementController::ActiveSettlementController = this;
}

// Called when the game starts or when spawned
void ASettlementController::BeginPlay()
{
	Super::BeginPlay();

	InitSettlements();
	UE_LOG(LogTemp, Warning, TEXT("The Settlement Controller Instance is %p"), this);

	SetupConsoleCommands();
	bDebugConsole = false;
}

void ASettlementController::BeginDestroy()
{
	Super::BeginDestroy();

	// Clean up any dynamically allocated objects
	for (USettlement* Settlement : Settlements)
	{
		if (Settlement && Settlement->IsValidLowLevel())
		{
			Settlement->ConditionalBeginDestroy();  // Properly destroy the actor
		}
	}

	// Clear the array after destroying the actors
	Settlements.Empty();

	UE_LOG(LogTemp, Warning, TEXT("SettlementController destroyed."));
}

// Called every frame
void ASettlementController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDebugScreen)
	{
		DebugSettlements();
	}
}

void ASettlementController::InitSettlements()
{
	if (SettlementDataTable)
	{
		static const FString ContextString(TEXT("Settlement Context"));

		// Get all rows of the FSettlementData structure
		TArray<FSettlementData*> AllRows;
		SettlementDataTable->GetAllRows<FSettlementData>(ContextString, AllRows);

		// Iterate through all rows
		int8 i = 4;
		for (FSettlementData* SettlementRow : AllRows)
		{
			if (SettlementRow)
			{
				if (bDebugConsole == true) {
					UE_LOG(LogTemp, Warning, TEXT("Settlement Name: %s"), *SettlementRow->Name.ToString());
					UE_LOG(LogTemp, Warning, TEXT("Settlement Description: %s"), *SettlementRow->Description.ToString());
					UE_LOG(LogTemp, Warning, TEXT("Price Multiple: %f"), SettlementRow->PriceMultipleBy);
				}
				// Initialize or store settlement as needed
				USettlement* NewSettlement = NewObject<USettlement>(this);
				NewSettlement->InitData(SettlementRow);
				NewSettlement->SetName(SettlementRow->Name.ToString());
				NewSettlement->PrimaryComponentTick.bCanEverTick = true;
				NewSettlement->setScreenDebug(i);
				if (!this->bDebugScreen)
				{
					NewSettlement->bDebug = false;
				}

				Settlements.Add(NewSettlement);

				NewSettlement->RegisterComponent();
			}
		}
		activeSettlementCount = Settlements.Num();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SettlementDataTable is not loaded!"));
	}
}

void ASettlementController::SettlementReputationUpdate()
{
	for (USettlement* Settle : Settlements)
	{
		Settle->SetRenownBy((Settle->GetRenownBy() + CalculateRenownForSettlement(Settle)));
	}
}

void ASettlementController::CookDistanceData()
{
	for (int i = 0; i < Settlements.Num(); i++)
	{
		for (USettlement* IteratorSettlement : Settlements)
		{
			if (Settlements[i]->GetName() == IteratorSettlement->GetName())
				continue;
			DistanceCache[Settlements[i]->GetName()]
				[IteratorSettlement->GetName()] = FVector2D::Distance(
					*Settlements[i]->GetLocation(), *IteratorSettlement->GetLocation()
				);
		}
	}
}

void ASettlementController::DebugSettlements()
{
	if (GEngine)
	{
		// Iterate through all settlements
		for (int32 i = 0; i < Settlements.Num(); i++)
		{
			USettlement* CurrentSettlement = Settlements[i];
			if (CurrentSettlement)
			{
				// Fetch the relevant data from each settlement
				FString Name = CurrentSettlement->GetName();
				FString Description = CurrentSettlement->GetDescription();
				float Renown = CurrentSettlement->GetRenownBy();
				float Reputation = CurrentSettlement->GetReputation();

				// Create formatted debug messages for each property
				FString SettlementHeader = FString::Printf(TEXT("Settlement %d:"), i);
				FString SettlementName = FString::Printf(TEXT("%s:"), *Name);
				FString SettlementDescription = FString::Printf(TEXT("Description: %s"), *Description);
				FString SettlementRenown = FString::Printf(TEXT("Renown: %.2f"), Renown);
				FString SettlementReputation = FString::Printf(TEXT("Reputation: %.2f"), Reputation);

				// Display each property on a separate line with a small delay so they appear together
				GEngine->AddOnScreenDebugMessage(i + 100, 5.0f, FColor::Yellow, SettlementName);
				GEngine->AddOnScreenDebugMessage(i + 200, 5.0f, FColor::Cyan, SettlementRenown);
				GEngine->AddOnScreenDebugMessage(i + 300, 5.0f, FColor::Green, SettlementReputation);
				GEngine->AddOnScreenDebugMessage(i + 400, 5.0f, FColor::Emerald, "=========================");
				// Add a separator for readability between settlements
			}
		}
	}
}

void ASettlementController::CalculateInflunceRate()
{
	for (USettlement* SettlementA : Settlements)
	{
		float InflunceRate = 1;
		int i = 0;  // Moved `i` out of the inner loop

		for (USettlement* SettlementB : Settlements)
		{
			// Skip if the settlement is the same
			if (SettlementA->GetName() == SettlementB->GetName())
				continue;

			// Check if the distance is within the limit
			if (SettlementA->DistanceMap.Contains(SettlementB->GetName()) &&
				SettlementA->DistanceMap[SettlementB->GetName()] <= MAX_SETTLEMENT_DISTANCE)
			{
				// Add the influence rate based on distance
				InflunceRate += SettlementA->DistanceMap[SettlementB->GetName()] / MAX_SETTLEMENT_DISTANCE;
				i++;
			}
		}

		// Only divide if `i` is greater than 0 to avoid division by zero
		if (i > 0)
		{
			InflunceRate /= i;
		}

		// Set the influence rate for the settlement
		SettlementA->SetInflunceRate(InflunceRate);
	}
}

void ASettlementController::SetupConsoleCommands()
{
	static FAutoConsoleCommand ChangeReputationCmd(
		TEXT("Settlement.ChangeReputation"),
		TEXT("Changes the reputation of a specified settlement.\nUsage: Settlement.ChangeReputation SettlementName NewReputation"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
			{
				if (Args.Num() == 2)
				{
					FString SettlementName = Args[0];
					float NewReputation = FCString::Atof(*Args[1]);

					// Access the static instance of ASettlementController
					if (ASettlementController::ActiveSettlementController)
					{
						ASettlementController::ActiveSettlementController->ChangeSettlementReputation(SettlementName, NewReputation);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("No active SettlementController instance."));
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Invalid arguments. Usage: Settlement.ChangeReputation SettlementName NewReputation"));
				}
			})
	);
}

float ASettlementController::CalculateRenownForSettlement(USettlement* Settlement)
{
	float renownIncrease = 0.f;
	for (USettlement* currentSett : Settlements)
	{
		if (currentSett->GetName() == Settlement->GetName())
		{
			continue;
		}
	}

	return renownIncrease /= (Settlements.Num() - 1);
}

void ASettlementController::ChangeSettlementReputation(FString SettlementName, float NewReputation)
{
	UE_LOG(LogTemp, Warning, TEXT("Command Function Work at %p"), this);
	// Iterate over all settlements to find the matching one
	for (USettlement* Settlement : Settlements)
	{
		UE_LOG(LogTemp, Warning, TEXT("Founded Settlement: %s"), *Settlement->GetName());
		if (Settlement && Settlement->GetName() == SettlementName)
		{
			Settlement->SetReputation(NewReputation);
			UE_LOG(LogTemp, Warning, TEXT("Reputation of %s set to %f"), *SettlementName, NewReputation);
			return;  // Exit once the settlement is found
		}
	}

	// If no matching settlement was found
	UE_LOG(LogTemp, Error, TEXT("Settlement with name %s not found"), *SettlementName);
}