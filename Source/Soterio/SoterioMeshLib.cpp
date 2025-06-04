/*
Just remember:
	Everything is possible.
	But some things need to be done.
*/

#include "SoterioMeshLib.h"
#include <Serialization/BufferArchive.h>
#include "Engine/StaticMesh.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "AssetRegistry/AssetRegistryModule.h"

FORCEINLINE void USoterioMeshLib::CalculateNormals(FProductProperties* Product)
{
	if (!Product)
	{
		UE_LOG(LogTemp, Error, TEXT("Product is empty"));
	}

	Product->Normals.SetNumZeroed(Product->Vertices.Num());

	// Temporary array to accumulate normals
	TArray<int32> NormalCounts;
	NormalCounts.SetNumZeroed(Product->Vertices.Num());

	for (int i = 0; i < Product->Triangles.Num(); i += 3)
	{
		int32 Index0 = Product->Triangles[i];
		int32 Index1 = Product->Triangles[i + 1];
		int32 Index2 = Product->Triangles[i + 2];

		FVector3f Vertex0 = Product->Vertices[Index0];
		FVector3f Vertex1 = Product->Vertices[Index1];
		FVector3f Vertex2 = Product->Vertices[Index2];

		FVector3f Edge1 = Vertex1 - Vertex0;
		FVector3f Edge2 = Vertex2 - Vertex0;

		FVector3f FaceNormal = FVector3f::CrossProduct(Edge2, Edge1).GetSafeNormal();

		Product->Normals[Index0] += FaceNormal;
		Product->Normals[Index1] += FaceNormal;
		Product->Normals[Index2] += FaceNormal;

		NormalCounts[Index0]++;
		NormalCounts[Index1]++;
		NormalCounts[Index2]++;
	}

	for (int i = 0; i < Product->Normals.Num(); i++)
	{
		if (NormalCounts[i] > 0)
		{
			Product->Normals[i] /= NormalCounts[i];  // Average the normal
			Product->Normals[i].Normalize();         // Ensure it's a unit vector
		}
	}
}

void USoterioMeshLib::CalculateTangents(FProductProperties* Product)
{
	if (!Product)
	{
		UE_LOG(LogTemp, Error, TEXT("Product is empty"));
	}
	Product->Tangents.SetNumZeroed(Product->Vertices.Num());

	for (int i = 0; i < Product->Triangles.Num(); i += 3)
	{
		int32 Index0 = Product->Triangles[i];
		int32 Index1 = Product->Triangles[i + 1];
		int32 Index2 = Product->Triangles[i + 2];

		FVector3f Vertex0 = Product->Vertices[Index0];
		FVector3f Vertex1 = Product->Vertices[Index1];
		FVector3f Vertex2 = Product->Vertices[Index2];

		FVector3f Edge1 = Vertex1 - Vertex0;
		FVector3f Edge2 = Vertex2 - Vertex0;

		// Get the current face normal
		FVector3f FaceNormal = Product->Normals[Index0];  // Assuming flat normals per triangle

		// Calculate the tangent using the cross product with the face normal
		FVector3f Tangent = FVector3f::CrossProduct(FaceNormal, Edge2).GetSafeNormal();
		Product->Tangents[Index0] = Tangent;
		Product->Tangents[Index1] = Tangent;
		Product->Tangents[Index2] = Tangent;
	}
}

void USoterioMeshLib::RotateMesh(FProductProperties* Product, float RotationDegree, char Axis)
{
	if (!Product || Product->Vertices.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Product Array is empty or missing elements!"));
		return;
	}

	float Radians = FMath::DegreesToRadians(RotationDegree);
	float CosAngle = cos(Radians);
	float SinAngle = sin(Radians);

	for (int i = 0; i < Product->Vertices.Num(); i++)
	{
		float X = Product->Vertices[i].X;
		float Y = Product->Vertices[i].Y;
		float Z = Product->Vertices[i].Z;

		switch (Axis)
		{
		case 'X':  // Rotate around X-axis
			Product->Vertices[i].Y = Y * CosAngle - Z * SinAngle;
			Product->Vertices[i].Z = Y * SinAngle + Z * CosAngle;
			break;

		case 'Y':  // Rotate around Y-axis
			Product->Vertices[i].X = X * CosAngle + Z * SinAngle;
			Product->Vertices[i].Z = Z * CosAngle - X * SinAngle;
			break;

		case 'Z':  // Rotate around Z-axis
			Product->Vertices[i].X = X * CosAngle - Y * SinAngle;
			Product->Vertices[i].Y = X * SinAngle + Y * CosAngle;
			break;

		default:
			UE_LOG(LogTemp, Warning, TEXT("Invalid axis specified. Use 'X', 'Y', or 'Z'."));
			return;
		}
	}
	for (FVector& Spt : Product->SplinePoints)
	{
		float X = Spt.X;
		float Y = Spt.Y;
		float Z = Spt.Z;

		switch (Axis)
		{
		case 'X':
			Spt.Y = Y * CosAngle - Z * SinAngle;
			Spt.Z = Y * SinAngle + Z * CosAngle;
		case 'Y':
			Spt.X = X * CosAngle + Z * SinAngle;
			Spt.Z = Z * CosAngle - X * SinAngle;
		case 'Z':
			Spt.X = X * CosAngle - Y * SinAngle;
			Spt.Y = X * SinAngle + Y * CosAngle;
		default:
			break;
		}
	}

	USoterioMeshLib::CalculateSmoothNormals(Product, 5);
	USoterioMeshLib::CalculateTangents(Product);
}

void USoterioMeshLib::ExtractMeshData(UStaticMesh* BaseMesh, FProductProperties& OutProductProperties, bool bConsoleDebug)
{

	if (!BaseMesh || !BaseMesh->GetRenderData() || BaseMesh->GetRenderData()->LODResources.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("There is no base mesh or LOD resources are missing!"));
		return;
	}
	BaseMesh->bAllowCPUAccess = true;

	const FStaticMeshLODResources& LODResources = BaseMesh->GetRenderData()->LODResources[0];
	const FPositionVertexBuffer& VertexBuffer = LODResources.VertexBuffers.PositionVertexBuffer;
	const FRawStaticIndexBuffer& IndexBuffer = LODResources.IndexBuffer;
	const FStaticMeshVertexBuffer& StaticMeshVertexBuffer = LODResources.VertexBuffers.StaticMeshVertexBuffer;

	OutProductProperties.ProductID = FGuid::NewGuid();
	OutProductProperties.Vertices.Empty();
	OutProductProperties.Triangles.Empty();
	OutProductProperties.Normals.Empty();
	OutProductProperties.UVs.Empty();
	OutProductProperties.Tangents.Empty();
	OutProductProperties.VertexHeat.Empty();

	for (uint32 i = 0; i < VertexBuffer.GetNumVertices(); i++)
	{
		FVector3f Position = VertexBuffer.VertexPosition(i);
		OutProductProperties.Vertices.Add(Position);
		OutProductProperties.VertexHeat.Add(20);
	}

	if (bConsoleDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("Vertex count is :%d"), OutProductProperties.Vertices.Num());
	}

	int32 NumIndices = IndexBuffer.GetNumIndices();
	for (int32 i = 0; i < NumIndices; i++)
	{
		int32 Index = IndexBuffer.GetIndex(i);
		OutProductProperties.Triangles.Add(Index);
	}

	for (uint32 i = 0; i < VertexBuffer.GetNumVertices(); i++)
	{
		FVector3f Normal = StaticMeshVertexBuffer.VertexTangentZ(i);
		FVector3f Tangent = StaticMeshVertexBuffer.VertexTangentX(i);
		FVector2f UV = StaticMeshVertexBuffer.GetVertexUV(i, 0);

		OutProductProperties.Normals.Add(Normal);
		OutProductProperties.Tangents.Add(Tangent);
		OutProductProperties.UVs.Add(UV);
	}
	OutProductProperties.GenerateSplineData();
}

URealtimeMeshSimple* USoterioMeshLib::CreateOreInstance(UStaticMesh* BaseStaticMesh,
	URealtimeMeshComponent* RealtimeMeshComponent, const FProductProperties& ProductProperties, UMaterialInterface* ProductMaterial, bool bConsoleDebug)
{
	if (!BaseStaticMesh || !RealtimeMeshComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("BaseStaticMesh or RealtimeMeshComponent is not set!"));
		return nullptr;
	}

	URealtimeMeshSimple* RealtimeMesh = RealtimeMeshComponent->InitializeRealtimeMesh<URealtimeMeshSimple>();
	if (!RealtimeMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize RealtimeMesh."));
		return nullptr;
	}

	// Set up the stream set and builder
	FRealtimeMeshStreamSet StreamSet;
	TRealtimeMeshBuilderLocal<uint16, FPackedNormal, FVector2DHalf, 1> Builder(StreamSet);

	Builder.EnableTangents();
	Builder.EnableTexCoords();
	Builder.EnableColors();
	Builder.EnablePolyGroups();

	int debugIter = 0;
	for (int i = 0; i + 2 < ProductProperties.Triangles.Num(); i += 3)
	{
		int Index0 = ProductProperties.Triangles[i];
		int Index1 = ProductProperties.Triangles[i + 1];
		int Index2 = ProductProperties.Triangles[i + 2];

		if (Index0 >= ProductProperties.Vertices.Num() ||
			Index1 >= ProductProperties.Vertices.Num() ||
			Index2 >= ProductProperties.Vertices.Num())
		{
			UE_LOG(LogTemp, Error, TEXT("Triangle index out of bounds! Indices: %d, %d, %d"), Index0, Index1, Index2);
			continue;
		}

		int V0 = Builder.AddVertex(ProductProperties.Vertices[Index0])
			.SetNormal(ProductProperties.Normals[Index0])
			.SetTexCoord(ProductProperties.UVs[Index0])
			.SetTangent(ProductProperties.Tangents[Index0]);

		int V1 = Builder.AddVertex(ProductProperties.Vertices[Index1])
			.SetNormal(ProductProperties.Normals[Index1])
			.SetTexCoord(ProductProperties.UVs[Index1])
			.SetTangent(ProductProperties.Tangents[Index1]);

		int V2 = Builder.AddVertex(ProductProperties.Vertices[Index2])
			.SetNormal(ProductProperties.Normals[Index2])
			.SetTexCoord(ProductProperties.UVs[Index2])
			.SetTangent(ProductProperties.Tangents[Index2]);

		Builder.AddTriangle(V0, V1, V2, 0);
		debugIter++;
	}
	if (bConsoleDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("Instanced Triangle count: %d"), debugIter);
	}

	RealtimeMesh->SetupMaterialSlot(0, "PrimaryMaterial");
	const FRealtimeMeshSectionGroupKey GroupKey = FRealtimeMeshSectionGroupKey::Create(0, FName("Test"));
	const FRealtimeMeshSectionKey PolyGroup0SectionKey = FRealtimeMeshSectionKey::CreateForPolyGroup(GroupKey, 0);

	RealtimeMesh->CreateSectionGroup(GroupKey, StreamSet);
	RealtimeMesh->UpdateSectionConfig(PolyGroup0SectionKey, FRealtimeMeshSectionConfig(0));

	RealtimeMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RealtimeMeshComponent->SetMaterial(0, ProductMaterial);
	RealtimeMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	RealtimeMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

	return RealtimeMesh;
}

inline static void FlatHammerShape(FProductProperties& ProductProperties, FHammerData& Hammer, FHitResult Hit, bool bDebug)
{
	FVector3f HitLocation = FVector3f(Hit.Location);

	FVector3f Local = FVector3f(Hit.Component->GetComponentTransform().InverseTransformPosition(FVector(HitLocation)));
	FVector3f ImpactNormal = FVector3f(0, 0, 1);
	int i = 0;
	float StressBound = ((ProductProperties.MaxLength - ProductProperties.Length) / ProductProperties.MaxLength);
	for (FVector3f& Vert : ProductProperties.Vertices)
	{
		FVector3f Deform = ImpactNormal * (0.1f * (ProductProperties.VertexHeat[i] / 1440)) * Vert.Z;
		Vert -= Deform;
		if (ProductProperties.bIsMaxLength == false)
		{
			float HeatEffect = ProductProperties.VertexHeat[i] / 1440;
			float StressEffect = StressBound / 3;
			Vert.Y *= FMath::Pow(1 + HeatEffect * StressEffect, 0.07f);
			Vert.X *= FMath::Pow(1 + HeatEffect * StressEffect, 0.03);
		}
		i++;
	}
}

inline static void RoundHammerShape(FProductProperties& ProductProperties, FHammerData& Hammer, FHitResult Hit, bool bDebug)
{
	FVector3f HitLocation = FVector3f(Hit.Location);
	UE_LOG(LogTemp, Warning, TEXT("RoundHammer Called"));
	FVector3f Local = FVector3f(Hit.Component->GetComponentTransform().InverseTransformPosition(FVector(HitLocation)));
	FVector3f ImpactNormal = FVector3f(Hit.ImpactNormal);

	const float InvMaxRadius = 1.0f / Hammer.MaxRadius;
	const float MaxRadiusSquared = Hammer.MaxRadius * Hammer.MaxRadius;

	int AffectedVert = 0;
	for (FVector3f& Vert : ProductProperties.Vertices)
	{
		FVector3f Delta = Vert - Local;
		float DistSquared = Delta.SizeSquared();
		if (DistSquared < MaxRadiusSquared && Vert.Z > 0)
		{
			float falloff = 1.0f - FMath::Sqrt(DistSquared) * InvMaxRadius;
			FVector3f Deform = ImpactNormal * 0.1f * falloff * Vert.Z;
			Vert -= Deform;
			AffectedVert++;
		}
	}


	if (bDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("Affected vertices: %d"), AffectedVert);
	}
}

void USoterioMeshLib::ModifyMesh(FProductProperties& ProductProperties, FHammerData& Hammer, FHitResult Hit, bool bDebug)
{
	if (!Hit.bBlockingHit)
	{
		UE_LOG(LogTemp, Error, TEXT("Hit point is not valid!"));
		return;
	}

	FVector3f HitLocation = FVector3f(Hit.Location);

	FVector3f Local = FVector3f(Hit.Component->GetComponentTransform().InverseTransformPosition(FVector(HitLocation)));
	FVector3f ImpactNormal = FVector3f(Hit.ImpactNormal);

	//UE_LOG(LogTemp, Warning, TEXT("Impact Point in Local Coordinates: X: %f, Y: %f, Z: %f"),
	//	Local.X, Local.Y, Local.Z);

	ProductProperties.GenerateSplineData();
	switch (Hammer.Face_0)
	{
	case ES_HAMMER_SHAPE::FLAT:
		FlatHammerShape(ProductProperties, Hammer, Hit, bDebug);
		break;
	case ES_HAMMER_SHAPE::ROUND:
		RoundHammerShape(ProductProperties, Hammer, Hit, bDebug);
		break;
	case ES_HAMMER_SHAPE::SHARP:
		UE_LOG(LogTemp, Error, TEXT("Not Prepared!"));
		break;
	default:
		break;
	}
	//const float InvMaxRadius = 1.0f / Hammer.MaxRadius;
	//const float MaxRadiusSquared = Hammer.MaxRadius * Hammer.MaxRadius;
	//const float spreadMultipler = 0.01;
	//const float SpreadAmount = Hammer.Weigth * Hammer.Size * spreadMultipler ;
	//const FVector3f Spread = FVector3f((SpreadAmount / spreadMultipler), 1.0f, (SpreadAmount/ spreadMultipler));



	//int AffectedVert = 0;
	//for (FVector3f& Vert : ProductProperties.Vertices)
	//{
	//	FVector3f Delta = Vert - Local;
	//	float DistSquared = Delta.SizeSquared();
	//	if (DistSquared < MaxRadiusSquared)
	//	{
	//		float falloff = 1.0f - FMath::Sqrt(DistSquared) * InvMaxRadius;
	//		FVector3f Deform = ImpactNormal * 0.1f * falloff;
	//		Vert -= Deform;
	//		AffectedVert++;
	//	}
	//}
	//UE_LOG(LogTemp, Warning, TEXT("Total Deformed Vertices: %d"), AffectedVert);
}


float USoterioMeshLib::Expansion(float fallof, FHitResult Hit)
{
	return 0.0f;
}

bool USoterioMeshLib::SaveMeshProperties(FProductProperties& Product, const FString& FilePath)
{
	FBufferArchive Archive;
	Product.Serialize(Archive);

	if (FFileHelper::SaveArrayToFile(Archive, *FilePath))
	{
		Archive.FlushCache();
		Archive.Empty();
		return true;
	}

	Archive.FlushCache();
	Archive.Empty();
	return false;
}

bool USoterioMeshLib::LoadMeshProperties(FProductProperties& Product, const FString& FilePath)
{
	TArray<uint8> BinaryArray;
	if (!FFileHelper::LoadFileToArray(BinaryArray, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Unable read Mesh Properties from %s"), *FilePath);
		return false;
	}

	FMemoryReader Reader(BinaryArray, true);
	Reader.Seek(0);
	UE_LOG(LogTemp, Warning, TEXT("LOL"));

	Product.Serialize(Reader);
	return false;
}

void FlatForge()
{

}

void USoterioMeshLib::CalculateSmoothNormals(FProductProperties* Product, int Depth)
{
	for (int i = 0; i < Depth; i++)
	{
		USoterioMeshLib::CalculateNormals(Product);
	}
}

void USoterioMeshLib::UpdateHeat(FProductProperties& Product, float Heat)
{
	const float MaxHeat = 1440.0f;
	float HeatFactor = 0.8f;
	const float MaxDistance = 19.5f;

	for (int i = 0; i < Product.Vertices.Num(); i++)
	{
		if (Product.VertexHeat[i] <= 250)
		{
			HeatFactor = 1.2;
		}
		else if (Product.VertexHeat[i] <= 450)
		{
			HeatFactor = 0.75;
		}
		else if (Product.VertexHeat[i] <= 650)
		{
			HeatFactor = 0.60;
		}
		else if (Product.VertexHeat[i] <= 700)
		{
			HeatFactor = 1.4;
		}
		else if (Product.VertexHeat[i] <= 950)
		{
			HeatFactor = 0.6;
		}
		else {
			HeatFactor = 1;
		}

		float Distance = FMath::Clamp(FVector3f::Dist(Product.Vertices[i], FVector3f(0, -9, 0)), 0, MaxDistance);
		Product.VertexHeat[i] += (MaxDistance - Distance) * HeatFactor;
		if (Product.VertexHeat[i] > MaxHeat)
		{
			Product.VertexHeat[i] = MaxHeat;
		}
		Product.Vertices[i] *= FVector3f(1.0003, 0.9996, 1.0003);
	}
}

void USoterioMeshLib::DecreaseHeat(FProductProperties& Product)
{
	for (int i = 0; i < Product.Vertices.Num(); i++)
	{
		Product.VertexHeat[i] -= (Product.VertexHeat[i] / 5760) * ((FVector3f::Dist(FVector3f(0, 0, 0), Product.Vertices[i])) / 6);
		if (Product.VertexHeat[i] <= 0)
		{
			Product.VertexHeat[i] = 0;
		}
	}
}

FColor USoterioMeshLib::GenerateVertexColor(float Heat)
{
	return FColor(FMath::Lerp(0, 255, (Heat / 1440)), 0, 0);
}

UStaticMesh* USoterioMeshLib::ConvertToStaticMesh(
	UObject* Outer,
	const TArray<FVector3f>& Vertices,
	const TArray<int32>& Triangles,
	const TArray<FVector3f>& Normals,
	const TArray<FVector2f>& UVs)
{
	UStaticMesh* NewMesh = NewObject<UStaticMesh>(Outer, NAME_None, RF_Public | RF_Standalone);

	FMeshDescription MeshDescription;
	FStaticMeshAttributes Attributes(MeshDescription);
	Attributes.Register();

	TArray<FVertexID> VertexIDs;
	for (const FVector3f& Vertex : Vertices)
	{
		VertexIDs.Add(MeshDescription.CreateVertex());
		Attributes.GetVertexPositions()[VertexIDs.Last()] = Vertex;
	}

	for (int32 i = 0; i < Triangles.Num(); i += 3)
	{
		FPolygonGroupID PolygonGroup = MeshDescription.CreatePolygonGroup();
		TArray<FVertexInstanceID> VertexInstanceIDs;

		for (int32 j = 0; j < 3; ++j)
		{
			FVertexInstanceID VertexInstanceID = MeshDescription.CreateVertexInstance(VertexIDs[Triangles[i + j]]);
			Attributes.GetVertexInstanceUVs().Set(VertexInstanceID, 0, UVs[i + j]); // Check again
			Attributes.GetVertexInstanceNormals()[VertexInstanceID] = Normals[i + j];
			VertexInstanceIDs.Add(VertexInstanceID);
		}

		MeshDescription.CreatePolygon(PolygonGroup, VertexInstanceIDs);
	}

	FStaticMeshLODResources& LODResources = NewMesh->GetRenderData()->LODResources[0];

	NewMesh->BuildFromMeshDescription(MeshDescription, LODResources);

	FAssetRegistryModule::AssetCreated(NewMesh);

	return NewMesh;
}


void USoterioMeshLib::AlignCenter(FProductProperties* Product, bool AlignHorizontal)
{
	if (!Product || Product->Vertices.Num() == 0)
		return;

	FVector3f Center = FVector3f::ZeroVector;

	for (int i = 0; i < Product->Vertices.Num(); i++)
	{
		Center += Product->Vertices[i];
	}
	Center /= Product->Vertices.Num();

	for (int i = 0; i < Product->Vertices.Num(); i++)
	{
		if (AlignHorizontal)
		{
			Product->Vertices[i].X -= Center.X;
			Product->Vertices[i].Y -= Center.Y;
		}
		else
		{
			Product->Vertices[i].Z -= Center.Z;
		}
	}
}

void USoterioMeshLib::AlignRaw()
{

}

TObjectPtr<USplineComponent> USoterioMeshLib::GenerateSpline(FProductProperties& Product, URealtimeMeshComponent& Component)
{
	if (Product.SplinePoints.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Spline points not found"));
		return nullptr;
	}

	if (Product.Spline == NULL)
	{
		USplineComponent* NewSpline = NewObject<USplineComponent>(&Component);
		if (!NewSpline)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create spline component"));
			return nullptr;
		}

		NewSpline->AttachToComponent(&Component, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		NewSpline->RegisterComponent();
		NewSpline->ClearSplinePoints();
		for (const FVector& Point : Product.SplinePoints)
		{
			NewSpline->AddSplinePoint(Point, ESplineCoordinateSpace::Local, true);
		}

		NewSpline->UpdateSpline();
		UE_LOG(LogTemp, Log, TEXT("Spline generated with %d points"), Product.SplinePoints.Num());
		Product.Spline = NewSpline;
		return TObjectPtr<USplineComponent>(NewSpline);
	}
	else
	{
		Product.Spline->ClearSplinePoints();
		Product.GenerateSplineData();
		for (const FVector& Point : Product.SplinePoints)
		{
			Product.Spline->AddSplinePoint(Point, ESplineCoordinateSpace::Local, true);
		}
		Product.Spline->UpdateSpline();
		return TObjectPtr<USplineComponent>();
	}
}


bool USoterioMeshLib::CheckThickness(FProductProperties& Product)
{
	float MinZ = FLT_MAX;
	float MaxZ = -FLT_MAX;

	for (const FVector3f& Vert : Product.Vertices)
	{
		MinZ = FMath::Min(MinZ, Vert.Z);
		MaxZ = FMath::Max(MaxZ, Vert.Z);
	}

	return false;
}

void USoterioMeshLib::CheckMeshHealth(FProductProperties* Product)
{
	//for (int i = 0; i < Product->Triangles.Num(); i += 3)
	//{
	//	FVector3f P0 = Product->Vertices[i];
	//	FVector3f P1 = Product->Vertices[i + 1];
	//	FVector3f P2 = Product->Vertices[i + 2];
	//	if (IsDegenerateTriangle(P0, P1, P2))
	//	{
	//		FixDegenerateTriangles(*Product);
	//		break;
	//	}
	//}
}

bool USoterioMeshLib::IsDegenerateTriangle(const FVector3f& P0, const FVector3f& P1, const FVector3f& P2, float Threshold = 0.0001f)
{
	FVector3f Edge1 = P1 - P0;
	FVector3f Edge2 = P2 - P0;
	FVector3f CrossProduct = FVector3f::CrossProduct(Edge1, Edge2);

	float Area = CrossProduct.Size();
	return Area < Threshold;
}

/// TODO: Write Re-Triangulate funct
void USoterioMeshLib::FixDegenerateTriangles(FProductProperties& ProductProperties)
{
	TArray<int32> TrianglesToRemove;

	for (int32 i = 0; i < ProductProperties.Triangles.Num(); i += 3)
	{
		FVector3f P0 = ProductProperties.Vertices[ProductProperties.Triangles[i]];
		FVector3f P1 = ProductProperties.Vertices[ProductProperties.Triangles[i + 1]];
		FVector3f P2 = ProductProperties.Vertices[ProductProperties.Triangles[i + 2]];

		if (IsDegenerateTriangle(P0, P1, P2, 0.001f))
		{
			TrianglesToRemove.Add(i);
		}
	}

	int32 id = 0;
	for (int32 Index : TrianglesToRemove)
	{
		ProductProperties.Triangles.RemoveAt(Index, 3, false);
		id = Index;
	}
	UE_LOG(LogTemp, Warning, TEXT("%d Triangles Affected"), id);
}

bool USoterioMeshLib::WriteRealtimeMeshToOBJ(const FString& FilePath, const FProductProperties& Mesh)
{
	const FString TimeStamp = FDateTime::Now()
		.ToString(TEXT("%Y-%m-%d-%H-%M-%S"));
	const FString savedir = FilePath.IsEmpty() ?
		FPaths::Combine(
			TEXT("C:/SoterioObjects"),
			FString::Printf(TEXT("RealtimeMesh_%s.obj"), *TimeStamp)) :
		FilePath;
	const FString Directory = FPaths::GetPath(savedir);
	IFileManager::Get().MakeDirectory(*Directory, /*Tree=*/true);

	TArray<FString> Lines;
	Lines.Reserve(Mesh.Vertices.Num() * 3 / 2);	// rough guess to avoid reallocs

	// 1. vertices
	for (const FVector3f& V : Mesh.Vertices)
	{
		Lines.Add(FString::Printf(TEXT("v %s %s %s"),
			*FString::SanitizeFloat(V.X),
			*FString::SanitizeFloat(V.Y),
			*FString::SanitizeFloat(V.Z)));
	}

	// 2. texture coordinates (optional)
	const bool bHasUVs = Mesh.UVs.Num() == Mesh.Vertices.Num();
	if (bHasUVs)
	{
		for (const FVector2f& UV : Mesh.UVs)
		{
			// flip V so OBJ (bottom-left origin) matches UE (top-left)
			Lines.Add(FString::Printf(TEXT("vt %s %s"),
				*FString::SanitizeFloat(UV.X),
				*FString::SanitizeFloat(1.0f - UV.Y)));
		}
	}

	// 3. normals (optional)
	const bool bHasNormals = Mesh.Normals.Num() == Mesh.Vertices.Num();
	if (bHasNormals)
	{
		for (const FVector3f& N : Mesh.Normals)
		{
			Lines.Add(FString::Printf(TEXT("vn %s %s %s"),
				*FString::SanitizeFloat(N.X),
				*FString::SanitizeFloat(N.Y),
				*FString::SanitizeFloat(N.Z)));
		}
	}

	// 4. faces (triangles) — OBJ is 1-based
	const int32 NumTriangles = Mesh.Triangles.Num() / 3;
	for (int32 TriIdx = 0; TriIdx < NumTriangles; ++TriIdx)
	{
		const int32 V0 = Mesh.Triangles[TriIdx * 3] + 1;
		const int32 V1 = Mesh.Triangles[TriIdx * 3 + 1] + 1;
		const int32 V2 = Mesh.Triangles[TriIdx * 3 + 2] + 1;

		FString Face;
		if (bHasUVs && bHasNormals)
		{
			Face = FString::Printf(TEXT("f %d/%d/%d %d/%d/%d %d/%d/%d"),
				V0, V0, V0, V1, V1, V1, V2, V2, V2);
		}
		else if (bHasUVs)
		{
			Face = FString::Printf(TEXT("f %d/%d %d/%d %d/%d"),
				V0, V0, V1, V1, V2, V2);
		}
		else if (bHasNormals)
		{
			Face = FString::Printf(TEXT("f %d//%d %d//%d %d//%d"),
				V0, V0, V1, V1, V2, V2);
		}
		else
		{
			Face = FString::Printf(TEXT("f %d %d %d"), V0, V1, V2);
		}
		Lines.Add(Face);
	}

	// 5. write to disk
	return FFileHelper::SaveStringArrayToFile(Lines, *savedir);
}