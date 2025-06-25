// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "../../Plugins/RealtimeMeshComponent/Source/RealtimeMeshComponent/Public/RealtimeMeshComponent.h"
#include "../../Plugins/RealtimeMeshComponent/Source/RealtimeMeshComponent/Public/RealtimeMeshSimple.h"

#include "GameTypes.h"

#include "SoterioMeshLib.generated.h"

/*
  __
 / _|_   _  ___| | __  _ __ ___ (_) ___ _ __ ___  ___  ___  / _| |_
| |_| | | |/ __| |/ / | '_ ` _ \| |/ __| '__/ _ \/ __|/ _ \| |_| __|
|  _| |_| | (__|   <  | | | | | | | (__| | | (_) \__ \ (_) |  _| |_
|_|  \__,_|\___|_|\_\ |_| |_| |_|_|\___|_|  \___/|___/\___/|_|  \__|
*/

UCLASS()
class SOTERIO_API USoterioMeshLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static void CalculateNormals(FProductProperties* Product);

	static void CalculateTangents(FProductProperties* Product);

	static void RotateMesh(FProductProperties* Product, float RotationDegree, char Axis);

	static void ExtractMeshData(UStaticMesh* BaseMesh, FProductProperties& OutProductProperties, bool bConsoleDebug = false);

	static URealtimeMeshSimple* CreateOreInstance(UStaticMesh* BaseStaticMesh, 
		URealtimeMeshComponent* RealtimeMeshComponent, const FProductProperties& ProductProperties, UMaterialInterface* ProductMaterial, bool bConsoleDebug);

	static void ModifyMesh(FProductProperties& ProductProperties, FHammerData& Hammer, FHitResult Hit, bool bDebug = false);

	static float Expansion(float fallof, FHitResult Hit);

	static bool SaveMeshProperties(FProductProperties& Product, const FString& FilePath);
	static bool LoadMeshProperties(FProductProperties& Product, const FString& FilePath);

	static void CalculateSmoothNormals(FProductProperties* Product, int Depth);
	static void UpdateHeat(FProductProperties& Product, float Heat, FVector3f HeatPoint);
	static void DecreaseHeat(FProductProperties& Product);
	static FColor GenerateVertexColor(float Heat);
	static UStaticMesh* ConvertToStaticMesh(UObject* Outer, const TArray<FVector3f>& Vertices, const TArray<int32>& Triangles, const TArray<FVector3f>& Normals, const TArray<FVector2f>& UVs);

	static void AlignCenter(FProductProperties* Product, bool AlignHorizontal = false);

	static void AlignRaw();
	static void AlignSpline();

	static TObjectPtr<USplineComponent> GenerateSpline(FProductProperties& Product, URealtimeMeshComponent& Component);
	static bool CheckThickness(FProductProperties& Product, FVector3f Vertex);
	static void CheckMeshHealth(FProductProperties* Product);
	static bool IsDegenerateTriangle(const FVector3f& P0, const FVector3f& P1, const FVector3f& P2, float Threshold);
	static void FixDegenerateTriangles(FProductProperties& ProductProperties);
};
