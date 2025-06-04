// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugOverlay.h"

void UDebugOverlay::HammerDebugOverlay(FHammerData& Hammer)
{
#if WITH_EDITOR
    FString DebugText;
    DebugText += FString::Printf(TEXT("Hammer Debug:"));
    DebugText += FString::Printf(TEXT("Max Radius: %.2f\n"), Hammer.MaxRadius);
    DebugText += FString::Printf(TEXT("Weight: %.2f\n"), Hammer.Weigth);
    DebugText += FString::Printf(TEXT("Size: %.2f\n"), Hammer.Size);

    DebugText += FString::Printf(TEXT("Face 0 Shape: %s\n"), *UEnum::GetValueAsString(Hammer.Face_0));
    DebugText += FString::Printf(TEXT("Face 1 Shape: %s\n"), *UEnum::GetValueAsString(Hammer.Face_1));

    if (Hammer.Mesh)
    {
        DebugText += FString::Printf(TEXT("Mesh: %s\n"), *Hammer.Mesh->GetName());
    }
    else
    {
        DebugText += TEXT("Mesh: None\n");
    }

    if (GEngine)
    {
        int32 Key = 0;
        float DisplayTime = 5.0f;
        FColor DisplayColor = FColor::Cyan;

        GEngine->AddOnScreenDebugMessage(Key, DisplayTime, DisplayColor, DebugText);
    }
#endif
}

void UDebugOverlay::HeatDebugOverlay(uint8 margin)
{
    FString Debug;
    Debug += FString::Printf(TEXT("Average Heat of product : %d"), margin);

    if (GEngine)
    {
        int32 Key = 5;
        float DisplayTime = 1.0f;
        FColor DisplayColor = FColor::Magenta;

        GEngine->AddOnScreenDebugMessage(Key, DisplayTime, DisplayColor, Debug);
    }
}

