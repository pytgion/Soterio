// Fill out your copyright notice in the Description page of Project Settings.


#include "S_Material.h"

SB_Material::SB_Material() {}
SB_Material::~SB_Material() {}

void SB_Material::SetId()
{
    Id = FGuid::NewGuid();
}

FGuid SB_Material::GetId() const
{
    return Id;
}

void SB_Material::SetName(FName IName)
{
    Name = IName;
}

FName SB_Material::GetName() const
{
    return Name;
}

void SB_Material::SetDescription(FText Descr)
{
    Description = Descr;
}

FText SB_Material::GetDescription() const
{
    return Description;
}

void SB_Material::SetAmount(float InAmount)
{
    Amount = FMath::Clamp(InAmount, 0.0f, MaxStackSize);
}

float SB_Material::GetAmount() const
{
    return Amount;
}

void SB_Material::SetMaxStackSize(float InMaxStackSize)
{
    MaxStackSize = InMaxStackSize;
}

float SB_Material::GetMaxStackSize() const
{
    return MaxStackSize;
}

void SB_Material::SetDensity(float InDensity)
{
    Density = InDensity;
}

float SB_Material::GetDensity() const
{
    return Density;
}

void SB_Material::SetMeltingPoint(uint16 InMeltingPoint)
{
    MeltingPoint = InMeltingPoint;
}

uint16 SB_Material::GetMeltingPoint() const
{
    return MeltingPoint;
}

bool SB_Material::IsAvailable() const
{
    return Amount > 0;
}
