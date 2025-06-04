// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameTypes.h"
#include "DebugOverlay.generated.h"

/**
 * 
 */
UCLASS()
class SOTERIO_API UDebugOverlay : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void HammerDebugOverlay(FHammerData& Hammer);

	void HeatDebugOverlay(uint8 margin);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool HammerDebug;
};
