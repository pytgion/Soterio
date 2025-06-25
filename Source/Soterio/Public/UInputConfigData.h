// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UInputConfigData.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * 
 */
UCLASS()
class SOTERIO_API UUInputConfigData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputMappingContext* DefaultMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputMappingContext* AnvilMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputMappingContext* ForgeMapping;

/// Anvil Controls

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* IA_Anvil_Hit;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))

	UInputAction* IA_Anvil_HammerSwitch;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))

	UInputAction* IA_Anvil_HammerFace;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))

	UInputAction* IA_Anvil_Mode;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))

	UInputAction* IA_Anvil_Look;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))

	UInputAction* IA_Anvil_Exit;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))

	UInputAction* IA_Anvil_Clockwise;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))

	UInputAction* IA_Anvil_CounterClockwise;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))

	UInputAction* IA_Anvil_Forward;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))

	UInputAction* IA_Anvil_Backward;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))

	UInputAction* IA_Anvil_Selection;
};
