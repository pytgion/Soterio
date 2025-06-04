// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "RealtimeMeshComponent.h"
#include "SRealTimeMesh.generated.h"

/**
 */

USTRUCT(BlueprintType)
struct FSurfaceNode
{
	GENERATED_BODY()
	bool		bIsSurface;
	float		Heat;
	FVector3f	VertexPos;
};

//URealtimeMeshComponent, public URealtimeMeshSimple
UCLASS(ClassGroup = (Rendering, Common), HideCategories = (Object, Activation, "Components|Activation"), ShowCategories = (Mobility), Meta = (BlueprintSpawnableComponent))
class SOTERIO_API USRealTimeMesh : public UMeshComponent
{
	GENERATED_BODY()
private:
	TObjectPtr<URealtimeMeshComponent> ProductComponent;
	//URealtimeMeshSimple RMS;

	TArray<FSurfaceNode> Nodes;
	TArray<FVector3f> VertexPos;
	TArray<int32> Triangles1;
	TArray<FVector3f> Normals;
	TArray<FVector2f> UVs;
	TArray<FVector3f> Tangents;

	FIntVector GridDimensions;
	float GridSpacing = 10.f;

public:
	UFUNCTION(BlueprintCallable)
	void Initialize(AActor* actor, UStaticMesh* Base, UMaterialInterface* ProductMaterial);
	void GenerateNodeCloud(const FVector& Origin, const FIntVector& Dimensions, float Spacing);
	void MarkSurfaceNodes();
	void BuildSurfaceMesh();
	void GenerateMeshFromNodes(int32 GridWidth);
	UFUNCTION(BlueprintCallable)
	void UpdateSurfaceMesh() {
		GenerateNodeCloud(GetComponentLocation(), FIntVector(20, 20, 20), 10.f);
		MarkSurfaceNodes();
		BuildSurfaceMesh();
	}
};
