// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "AnvilInterface.generated.h"

/**
 * 
 */
UCLASS()
class SOTERIO_API UAnvilInterface : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isRound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Power;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* RoundImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D*	 FlatImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D*	 CurrentImage;

	UFUNCTION(BlueprintCallable)
	void SetImage()
	{
		if (!RoundImage || !FlatImage)
			return;

		if (isRound)
			CurrentImage = RoundImage;
		else
			CurrentImage = FlatImage;
	}
};
