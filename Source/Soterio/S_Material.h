#pragma once

#include "CoreMinimal.h"

/**
 *  Essential material properties and functions
 */
class SOTERIO_API SB_Material
{

public:
    SB_Material();
    ~SB_Material();

private:
    UPROPERTY()
    FGuid Id;

    UPROPERTY()
    FName Name;

    UPROPERTY()
    FText Description;

    UPROPERTY()
    float Amount;

    UPROPERTY()
    float MaxStackSize;

    UPROPERTY()
    float Density;

    UPROPERTY()
    uint16 MeltingPoint;

public:


public:
    void SetId();
    FGuid GetId() const;

    void SetName(FName IName);
    FName GetName() const;

    void SetDescription(FText Descr);
    FText GetDescription() const;

    void SetAmount(float InAmount);
    float GetAmount() const;

    void SetMaxStackSize(float InMaxStackSize);
    float GetMaxStackSize() const;

    void SetDensity(float InDensity);
    float GetDensity() const;

    void SetMeltingPoint(uint16 InMeltingPoint);
    uint16 GetMeltingPoint() const;

    bool IsAvailable() const;
};
