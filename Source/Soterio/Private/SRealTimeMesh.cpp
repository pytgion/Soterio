// Fill out your copyright notice in the Description page of Project Settings.


#include "SRealTimeMesh.h"
#include "../SoterioMeshLib.h"
#include "Engine/EngineTypes.h"
#include "RealtimeMeshSimple.h"
#include "RealtimeMeshLibrary.h"
#include "RealtimeMeshComponent.h"

void USRealTimeMesh::Initialize(AActor* actor, UStaticMesh* Base, UMaterialInterface* ProductMaterial) {
	ProductComponent = CreateDefaultSubobject<URealtimeMeshComponent>(TEXT("ProductComponent"));
	ProductComponent->SetGenerateOverlapEvents(false);
	ProductComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	ProductComponent->SetupAttachment(actor->GetRootComponent());
	ProductComponent->SetVisibility(true);

	TArray<FProductProperties*> ProductQuery;
	FProductProperties* NewProduct = new FProductProperties();
	if (Base)
	{
		USoterioMeshLib::ExtractMeshData(Base, *NewProduct, true);
		ProductQuery.Add(NewProduct);
	}
	else return;
	
	Triangles1 = NewProduct->Triangles;
	Normals = NewProduct->Normals;
	UVs = NewProduct->UVs;
	Tangents = NewProduct->Tangents;

	ProductComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProductComponent->SetMaterial(0, ProductMaterial);

	ProductComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ProductComponent->SetCollisionObjectType(ECC_Camera);
	ProductComponent->SetCollisionResponseToAllChannels(ECR_Block);

	URealtimeMeshSimple* RealtimeMesh = ProductComponent->InitializeRealtimeMesh<URealtimeMeshSimple>();

	FRealtimeMeshStreamSet StreamSet;

	TRealtimeMeshBuilderLocal<uint16, FPackedNormal, FVector2DHalf, 1> Builder(StreamSet);

	Builder.EnableTangents();
	Builder.EnableTexCoords();
	Builder.EnableColors();
	Builder.EnablePolyGroups();

	for (int i = 0; i + 2 < Triangles1.Num(); i += 3)
	{
		int Index0 = Triangles1[i];
		int Index1 = Triangles1[i + 1];
		int Index2 = Triangles1[i + 2];
		
		int V0 = Builder.AddVertex(NewProduct->Vertices[Index0])
			.SetNormal(NewProduct->Normals[Index0])
			.SetTexCoord(NewProduct->UVs[Index0])
			.SetTangent(NewProduct->Tangents[Index0])
			.SetColor(USoterioMeshLib::GenerateVertexColor(NewProduct->VertexHeat[Index0]));

		int V1 = Builder.AddVertex(NewProduct->Vertices[Index1])
			.SetNormal(NewProduct->Normals[Index1])
			.SetTexCoord(NewProduct->UVs[Index1])
			.SetTangent(NewProduct->Tangents[Index1])
			.SetColor(USoterioMeshLib::GenerateVertexColor(NewProduct->VertexHeat[Index1]));

		int V2 = Builder.AddVertex(NewProduct->Vertices[Index2])
			.SetNormal(NewProduct->Normals[Index2])
			.SetTexCoord(NewProduct->UVs[Index2])
			.SetTangent(NewProduct->Tangents[Index2])
			.SetColor(USoterioMeshLib::GenerateVertexColor(NewProduct->VertexHeat[Index2]));

		Builder.AddTriangle(V0, V1, V2, 0);
	}

	//RealtimeMesh->SetupMaterialSlot(0, "PrimaryMaterial");
	USoterioMeshLib::GenerateSpline(*ProductQuery[0], *ProductComponent);
	ProductQuery[0]->Spline->UpdateSpline();

	const FRealtimeMeshSectionGroupKey GroupKey = FRealtimeMeshSectionGroupKey::Create(0, FName("TestTriangle"));
	const FRealtimeMeshSectionKey PolyGroup0SectionKey = FRealtimeMeshSectionKey::CreateForPolyGroup(GroupKey, 0);

	RealtimeMesh->CreateSectionGroup(GroupKey, StreamSet);
	RealtimeMesh->UpdateSectionConfig(PolyGroup0SectionKey, FRealtimeMeshSectionConfig(0), true);
}

void USRealTimeMesh::GenerateNodeCloud(const FVector& Origin, const FIntVector& Dimensions, float Spacing)
{
	Nodes.Empty();
	GridDimensions = Dimensions;
	GridSpacing = Spacing;

	for (int32 z = 0; z < Dimensions.Z; z++)
	{
		for (int32 y = 0; y < Dimensions.Y; y++)
		{
			for (int32 x = 0; x < Dimensions.X; x++)
			{
				FVector WorldPos = Origin + FVector(x, y, z) * Spacing;

				FSurfaceNode Node;
				Node.VertexPos = FVector3f(WorldPos);
				Node.bIsSurface = false;

				Nodes.Add(Node);
			}
		}
	}
}

void USRealTimeMesh::MarkSurfaceNodes()
{
	int32 Index = 0;

	//NOTE its gonna calculated by world position for later. now we need fastest we can.
	for (int32 z = 0; z < GridDimensions.Z; z++)
	{
		for (int32 y = 0; y < GridDimensions.Y; y++)
		{
			for (int32 x = 0; x < GridDimensions.X; x++, Index++)
			{
				if (x == 0 || x == GridDimensions.X - 1 ||
					y == 0 || y == GridDimensions.Y - 1 ||
					z == 0 || z == GridDimensions.Z - 1)
				{
					Nodes[Index].bIsSurface = true;
				}
			}
		}
	}
}

void USRealTimeMesh::BuildSurfaceMesh()
{
	TArray<FVector3f> Positions;
	TArray<int32> Triangles;

	// Only add every 8th surface node to keep it lightweight
	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		if (!Nodes[i].bIsSurface)
			continue;

		Positions.Add(Nodes[i].VertexPos);

		// Simple dummy triangle per 3 points
		if (Positions.Num() >= 3)
		{
			int32 Count = Positions.Num();
			Triangles.Add(Count - 3);
			Triangles.Add(Count - 2);
			Triangles.Add(Count - 1);
		}
	}

	//// Feed to RealtimeMesh
	//FRealtimeMeshSimpleMeshData MeshData;
	//MeshData.Positions = Positions;
	//MeshData.Triangles = Triangles;

	//this->InitializeRealtimeMesh();
	//this->SetupLOD(0);
	//this->CreateMeshSection(0, MeshData, true, ERealtimeMeshSectionDrawType::Solid);
}

void USRealTimeMesh::GenerateMeshFromNodes(int32 GridWidth)
{
	if (Nodes.Num() == 0 || GridWidth <= 0) return;

	// Prepare mesh container
	FRealtimeMeshStreamSet StreamSet;

	// Using basic 1 UV channel, half-precision texcoords
	TRealtimeMeshBuilderLocal<uint16, FPackedNormal, FVector2DHalf, 1> Builder(StreamSet);

	Builder.EnableTangents();
	Builder.EnableTexCoords();
	Builder.EnableColors();
	Builder.EnablePolyGroups();

	TMap<int32, int32> NodeToVertexMap;

	// Step 1: Add vertices
	for (int32 Index = 0; Index < Nodes.Num(); ++Index)
	{
		if (!Nodes[Index].bIsSurface) continue;

		const FVector3f& Pos = Nodes[Index].VertexPos;
		int32 VertexIndex = Builder.AddVertex((FVector3f)Pos)
			.SetNormalAndTangent(FVector3f(0, 0, 1), FVector3f(1, 0, 0)) // default normals
			.SetColor(FColor::White)
			.SetTexCoord(FVector2f(0, 0)); // optional UVs

		NodeToVertexMap.Add(Index, VertexIndex);
	}

	// Step 2: Add triangles (simple grid-based connection)
	int32 GridHeight = Nodes.Num() / GridWidth;
	for (int32 y = 0; y < GridHeight - 1; ++y)
	{
		for (int32 x = 0; x < GridWidth - 1; ++x)
		{
			int32 i0 = y * GridWidth + x;
			int32 i1 = i0 + 1;
			int32 i2 = i0 + GridWidth;
			int32 i3 = i2 + 1;

			if (NodeToVertexMap.Contains(i0) &&
				NodeToVertexMap.Contains(i1) &&
				NodeToVertexMap.Contains(i2))
			{
				Builder.AddTriangle(NodeToVertexMap[i0], NodeToVertexMap[i2], NodeToVertexMap[i1], 0);
			}

			if (NodeToVertexMap.Contains(i1) &&
				NodeToVertexMap.Contains(i2) &&
				NodeToVertexMap.Contains(i3))
			{
				Builder.AddTriangle(NodeToVertexMap[i1], NodeToVertexMap[i2], NodeToVertexMap[i3], 0);
			}
		}
	}

	// Setup RMC section
	//auto RealtimeMeshI = InitializeRealtimeMesh<URealtimeMeshSimple>();
	//RealtimeMeshI->SetupMaterialSlot(0, "GeneratedMaterial");

	const FRealtimeMeshSectionGroupKey GroupKey = FRealtimeMeshSectionGroupKey::Create(0, FName("SurfaceGroup"));
	//const FRealtimeMeshSectionKey SectionKey = FRealtimeMeshSectionKey::CreateForPolyGroup(GroupKey, 0);

	//RealtimeMeshI->CreateSectionGroup(GroupKey, StreamSet);
	//RealtimeMeshI->UpdateSectionConfig(SectionKey, FRealtimeMeshSectionConfig(0));
}
