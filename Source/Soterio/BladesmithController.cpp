// Fill out your copyright notice in the Description page of Project Settings.

#include "BladesmithController.h"
#include "Kismet/KismetSystemLibrary.h"
#include <JsonObjectConverter.h>
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"

DEFINE_LOG_CATEGORY(LogBladesmithController);

// Sets default values
ABladesmithController::ABladesmithController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsCameraActive = true;
	
	CurrentMode = ES_GameMode::Default;

	if (CurrentHammer == nullptr)
	{
		InitHammerList();
	}

	FurnaceHeat = 30.0f;
	if (!BellowBlowAmount)
	{
		BellowBlowAmount = 120;
	}
	ForgeHeatMultipler = 1.0f;

	ProductComponent = CreateDefaultSubobject<URealtimeMeshComponent>(TEXT("ProductComponent"));
	ProductComponent->SetGenerateOverlapEvents(false);
	ProductComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	ProductComponent->SetupAttachment(RootComponent);
	ProductComponent->SetVisibility(true);

	if (!BaseStaticMesh)
	{
		BaseStaticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Game/untitled.untitled'"));
	}

	if (!HammerData)
	{
		HammerData = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/HammerData.HammerData'"));
		UE_LOG(LogBladesmithController, Log, TEXT("Filled HammerData in BladesmithController constructor"));
	}

	ProductQuery.Reserve(1);
}

void ABladesmithController::Bindings()
{
	// Bind Delegate shit
	if (Character)
	{
		Character->NewGameMode.AddUObject(this, &ABladesmithController::SwitchGameMode);
	}
	if (Character->NewGameMode.IsBound())
	{
		UE_LOG(LogBladesmithController, Log, TEXT("BoundCheck is valid."));
	}

	// Bladesmith Tools
	TArray<AActor*> ToolActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Tools"), ToolActors);
	if (ToolActors.IsEmpty())
	{
		UE_LOG(LogBladesmithController, Warning, TEXT("Found no tool actors"));
	}
	else
	{
		for (AActor* Actor : ToolActors)
		{
			if (Actor->Tags.Contains(FName("Anvil")))
			{
				UE_LOG(LogBladesmithController, Log, TEXT("Anvil Actor Found!"));
				AnvilActor = Actor;
			}
			else if (Actor->Tags.Contains(FName("Forge")))
			{
				UE_LOG(LogBladesmithController, Log, TEXT("Forge Actor Found!"));
				ForgeActor = Actor;
			}
			else if (Actor->Tags.Contains(FName("Workbench")))
			{
				UE_LOG(LogBladesmithController, Log, TEXT("Workbench Actor Found!"));
				WorkbenchActor = Actor;
			}
			else if (Actor->Tags.Contains(FName("Grind")))
			{
				UE_LOG(LogBladesmithController, Log, TEXT("Grind Actor Found!"));
				GrindActor = Actor;
			}
		}
	}

	// Cameras
	TArray<AActor*> CameraActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Camera"), CameraActors);
	if (CameraActors.IsEmpty())
	{
		UE_LOG(LogBladesmithController, Error, TEXT("Found no camera actors"));
	}
	else
	{
		for (AActor* Actor : CameraActors)
		{
			if (Actor->Tags.Contains(FName("Anvil")))
			{
				UE_LOG(LogBladesmithController, Log, TEXT("Anvil Camera Found!"));
				AnvilCamera = Cast<ACameraActor>(Actor);
			}
			else if (Actor->Tags.Contains(FName("Forge")))
			{
				UE_LOG(LogBladesmithController, Log, TEXT("Forge Camera Found!"));
				ForgeCamera = Cast<ACameraActor>(Actor);
			}
			else if (Actor->Tags.Contains(FName("AnvilMod")))
			{
				UE_LOG(LogBladesmithController, Log, TEXT("AnvilMod Camera Found"));
				AnvilModCamera = Cast<ACameraActor>(Actor);
			}
			else if (Actor->Tags.Contains(FName("Workbench")))
			{
				UE_LOG(LogBladesmithController, Log, TEXT("Workbench Camera Found!"));
				WorkbenchCamera = Cast<ACameraActor>(Actor);
			}
			else if (Actor->Tags.Contains(FName("Grind")))
			{
				UE_LOG(LogBladesmithController, Log, TEXT("Grind Camera Found!"));
				GrindCamera = Cast<ACameraActor>(Actor);
			}
		}
	}
}

void ABladesmithController::BeginPlay()
{
	Super::BeginPlay();
	DynamicMaterial = ProductComponent->CreateDynamicMaterialInstance(0);
	Character = Cast<ASoterioCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	FString SaveName = "Default";
	LoadGameProgress(SaveName);
	Player = GetWorld()->GetFirstPlayerController();
	if (!Player)
		return;
	Bindings();
	if (ProductQuery.Num() == 0)
	{
		ProductQuery.Add(new FProductProperties());
	}
	USoterioMeshLib::ExtractMeshData(BaseStaticMesh, *ProductQuery[0], true);
	//USoterioMeshLib::CreateOreInstance(BaseStaticMesh, ProductComponent, *ProductQuery[0], ProductMaterial, true);
	CreateOreInstance();
	InitHammerList();
	GetWorld()->GetTimerManager().SetTimer(
		GameProgressTimeHandle,
		this,
		&ABladesmithController::TimePasses,
		0.1f,
		true
	);
}

void ABladesmithController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentMode == ES_GameMode::Anvil)
	{
		PerformRaycastFromAnvilCamera();
	}

	if (bScreenDebug)
	{
		DebugOverlay->HammerDebugOverlay(*CurrentHammer);
	}

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		TestController(PC);
	}
}

void ABladesmithController::TestController(APlayerController* PC)
{
	if (PC->WasInputKeyJustPressed(EKeys::P))
	{
		LogWarning("P pressed");
		USoterioMeshLib::RotateMesh(ProductQuery[0], 90, 'Y');
		UpdateProduct();
	}

	if (PC->WasInputKeyJustPressed(EKeys::O))
	{
		USoterioMeshLib::RotateMesh(ProductQuery[0], 90, 'Z');
		LogWarning("O Pressed");
		UpdateProduct();
	}

	if (PC->WasInputKeyJustPressed(EKeys::I))
	{
		USoterioMeshLib::RotateMesh(ProductQuery[0], 90, 'X');
		LogWarning("I Pressed");
		UpdateProduct();
	}
	if (PC->WasInputKeyJustPressed(EKeys::RightMouseButton))
	{
		USoterioMeshLib::AlignCenter(ProductQuery[0], true);
		UpdateProduct();
		LogWarning("Align");
	}
	if (PC->WasInputKeyJustPressed(EKeys::U))
	{
		LogWarning("U Pressed");
		UpdateProduct();
	}

	if (PC->WasInputKeyJustPressed(EKeys::Y))
	{
		USoterioMeshLib::CalculateSmoothNormals(ProductQuery[0], 10);
		LogWarning("Y Pressed");
		UpdateProduct();
	}

	if (PC->WasInputKeyJustPressed(EKeys::SpaceBar))
	{
		//LogWarning("SpaceBar pressed");
		//SwitchGameMode(ES_GameMode::Anvil);
	}
	if (PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
	{
		if (CurrentMode == ES_GameMode::Anvil)
		{
			USoterioMeshLib::ModifyMesh(*ProductQuery[0], *CurrentHammer, PerformRaycastFromAnvilCamera());
			UpdateProduct(DefaultSmoothRate);
		}
		if (CurrentMode == ES_GameMode::Forge)
		{
			UE_LOG(LogBladesmithController, Warning, TEXT("Average heat is: %d"), ProductQuery[0]->_Debug_averageHeat());
			UpdateProduct(DefaultSmoothRate);
		}
		LogWarning("LeftMouseButton Pressed");
	}
	if (PC->WasInputKeyJustPressed(EKeys::C))
	{
		ProductComponent->SetWorldScale3D(FVector(5.0f, 5.0f, 5.0f));
		LogWarning("Scaling Up");
	}
	if (PC->WasInputKeyJustPressed(EKeys::F))
	{

	}
	if (PC->WasInputKeyJustPressed(EKeys::B))
	{
		FString SavePath = FPaths::ProjectSavedDir() + GameProgress.SaveName.ToString();
		USoterioMeshLib::SaveMeshProperties(*ProductQuery[0], SavePath);
		SaveGameProgress();
		UE_LOG(LogBladesmithController, Warning, TEXT("Saving game to %s"), *SavePath);
	}
	if (PC->WasInputKeyJustPressed(EKeys::V))
	{
		FString SavePath = FPaths::ProjectSavedDir() + GameProgress.SaveName.ToString();
		USoterioMeshLib::LoadMeshProperties(*ProductQuery[0], SavePath);
		LoadGameProgress(SavePath);
		UE_LOG(LogBladesmithController, Warning, TEXT("Load from %s"), *SavePath);
	}
	if (PC->WasInputKeyJustPressed(EKeys::Escape))
	{
		LogWarning("Escape Pressed");
		UWorld* World = GetWorld();
		APlayerController* PlayerController = World->GetFirstPlayerController();

		if (World && PlayerController)
		{
			UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Quit, true);
		}
	}
	if (PC->WasInputKeyJustPressed(EKeys::MouseScrollDown))
	{
		SwitchHammer(false);
	}
	if (PC->WasInputKeyJustPressed(EKeys::MouseScrollUp))
	{
		SwitchHammer(true);
	}
}

void ABladesmithController::SwitchHammer(bool bDirection)
{
	if (HammerList.Num() == 0)
	{
		UE_LOG(LogBladesmithController, Error, TEXT("Hammer list is empty or not initialized!"));
		return;
	}

	if (bDirection)
	{
		CurrentHammerIndex = (CurrentHammerIndex + 1) % HammerList.Num();
	}
	else
	{
		CurrentHammerIndex = (CurrentHammerIndex - 1 + HammerList.Num()) % HammerList.Num();
	}

	CurrentHammer = HammerList.operator[](CurrentHammerIndex);

	UE_LOG(LogBladesmithController, Log, TEXT("Switched to hammer at index: %d"), CurrentHammerIndex);
}

void ABladesmithController::SpawnDecal(FVector3f Location, ES_HAMMER_SHAPE Shape)
{

}

void ABladesmithController::LogWarning(FString Text)
{
	if (bConsoleDebug_)
	{
		UE_LOG(LogBladesmithController, Warning, TEXT("%s"), *Text);
	}
}

void ABladesmithController::LogError(FString Text)
{
	UE_LOG(LogBladesmithController, Error, TEXT("%s"), *Text);
}

void ABladesmithController::SwitchGameMode(FHitResult HitResult)
{
	UE_LOG(LogBladesmithController, Log, TEXT("Broadcast Recieved for Actor: %s"), *HitResult.GetActor()->GetFName().ToString());
	if (CurrentMode != ES_GameMode::Default)
	{
		UE_LOG(LogBladesmithController, Log, TEXT("Game mode switched default."));
		CurrentMode = ES_GameMode::Default;
		SwitchCamera();
		return;
	}
	if (HitResult.GetActor() == AnvilActor)
	{
		CurrentMode = ES_GameMode::Anvil;
		UE_LOG(LogBladesmithController, Log, TEXT("Switching Anvil Mode"));
		if (AnvilActor->GetRootComponent()->DoesSocketExist(FName("AnvilSocket")))
		{
			LogWarning("AnvilSocket Found!");
			ProductComponent->AttachToComponent(AnvilActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "AnvilSocket");
		}
		SwitchCamera();
	}
	else if (HitResult.GetActor() == ForgeActor)
	{
		CurrentMode = ES_GameMode::Forge;
		UE_LOG(LogBladesmithController, Log, TEXT("Switching Forge Mode"));
		SwitchCamera();
		if (ForgeActor->GetRootComponent()->DoesSocketExist(FName("ForgeSocket")))
		{
			LogWarning("ForgeSocket Found!");
			ProductComponent->AttachToComponent(ForgeActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "ForgeSocket");
		}
	}
	else if (HitResult.GetActor() == WorkbenchActor)
	{
		CurrentMode = ES_GameMode::Workbench;
		UE_LOG(LogBladesmithController, Log, TEXT("Switching Wokrbench Mode"));
		SwitchCamera();
	}	
	else if (HitResult.GetActor() == GrindActor)
	{
		CurrentMode = ES_GameMode::Grindstone;
		UE_LOG(LogBladesmithController, Log, TEXT("Switching Grind Mode"));
		if (GrindActor->GetRootComponent()->DoesSocketExist(FName("GrindSocket")))
		{
			LogWarning("Grind Socket Found!");
			ProductComponent->AttachToComponent(GrindActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GrindSocket");
		}
		SwitchCamera();
	}
}

void ABladesmithController::SwitchCamera()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogBladesmithController, Warning, TEXT("PlayerController not found."));
		return;
	}

	AActor* PlayerCharacter = PlayerController->GetPawn();
	if (!bIsCameraActive)
	{
		PlayerController->SetViewTargetWithBlend(PlayerCharacter, CameraBlendTime);
	}
	else
	{
		switch (CurrentMode)
		{
		case ES_GameMode::Default:
			PlayerController->SetViewTargetWithBlend(PlayerCharacter, CameraBlendTime);
			break;
		case ES_GameMode::Anvil:
			PlayerController->SetViewTargetWithBlend(AnvilCamera, CameraBlendTime);
			break;
		case ES_GameMode::Furnace:
			break;
		case ES_GameMode::Forge:
			PlayerController->SetViewTargetWithBlend(ForgeCamera, CameraBlendTime);
			break;
		case ES_GameMode::AnvilMode:
			PlayerController->SetViewTargetWithBlend(AnvilModCamera, CameraBlendTime);
			break;
		case ES_GameMode::Grindstone:
			PlayerController->SetViewTargetWithBlend(GrindCamera, CameraBlendTime);
			break;
		case ES_GameMode::Workbench:
			PlayerController->SetViewTargetWithBlend(WorkbenchCamera, CameraBlendTime);
			break;
		case ES_GameMode::Dialog:
			break;
		default:
			break;
		}
	}
	bIsCameraActive = !bIsCameraActive;
}

void ABladesmithController::InitHammerList()
{
	if (!HammerData)
	{
		HammerData = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/HammerData.HammerData'"));
		if (!HammerData)
		{
			LogError("HammerData is empty! Line 425");
		}
		return;
	}
	static const FString ContextString(TEXT("Hammer Context"));


	TArray<FHammerData*> AllRows;
	HammerData->GetAllRows(ContextString, AllRows);

	int i = 0;
	for (FHammerData* Hammer : AllRows)
	{
		i++;
		if (Hammer)
		{
			HammerList.Add(Hammer);
		}
	}
	UE_LOG(LogBladesmithController, Log, TEXT("%d Hammers added!"), i);
	CurrentHammer = HammerList[0];
}

void ABladesmithController::TimePasses()
{
	if (CurrentMode == ES_GameMode::Forge)
	{
		USoterioMeshLib::UpdateHeat(*ProductQuery[0], FurnaceHeat);
	}
	else
	{
	USoterioMeshLib::DecreaseHeat(*ProductQuery[0]);
	}
	UpdateProduct(DefaultSmoothRate);

	if (GameProgress.Date.TimeOfDay <= 300)
	{
		GameProgress.Date.TimeOfDay += 1;
	}
	else
	{
		UE_LOG(LogBladesmithController, Warning, TEXT("New day has started!"));
	}
}

void ABladesmithController::CreateOreInstance()
{
	FProductProperties* NewProduct = new FProductProperties();

	if (BaseStaticMesh)
	{
		USoterioMeshLib::ExtractMeshData(BaseStaticMesh, *NewProduct, true);
		ProductQuery.Add(NewProduct);
	}
	else
	{
		UE_LOG(LogBladesmithController, Error, TEXT("BaseStaticMesh is not set!"));
		return;
	}
	UE_LOG(LogBladesmithController, Log, TEXT("%s GUID Created"), *NewProduct->ProductID.ToString());

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


	for (int i = 0; i + 2 < NewProduct->Triangles.Num(); i += 3)
	{
		int Index0 = NewProduct->Triangles[i];
		int Index1 = NewProduct->Triangles[i + 1];
		int Index2 = NewProduct->Triangles[i + 2];

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

void ABladesmithController::HeatUpForge()
{
	FurnaceHeat += (ForgeHeatMultipler * BellowBlowAmount);
}

void ABladesmithController::SwitchDefaultMode()
{
	SwitchGameMode(FHitResult());
}

void ABladesmithController::UpdateProduct(int CalculateNormalDepth)
{
	URealtimeMeshSimple* RealtimeMesh = Cast<URealtimeMeshSimple>(ProductComponent->GetRealtimeMesh());
	if (!RealtimeMesh)
	{
		UE_LOG(LogBladesmithController, Error, TEXT("RealtimeMesh is not valid!"));
		return;
	}

	FProductProperties EditableMeshData = *ProductQuery[0];

	FRealtimeMeshStreamSet StreamSet;
	TRealtimeMeshBuilderLocal<uint16, FPackedNormal, FVector2DHalf, 1> Builder(StreamSet);

	Builder.EnableTangents();
	Builder.EnableTexCoords();
	Builder.EnableColors();
	Builder.EnablePolyGroups();
	Builder.EnableColors();
	ProductComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProductComponent->SetCollisionObjectType(ECC_Camera);
	ProductComponent->SetCollisionResponseToAllChannels(ECR_Block);

	if (CalculateNormalDepth)
	{
		USoterioMeshLib::CalculateSmoothNormals(&EditableMeshData, CalculateNormalDepth);
	}

	for (int i = 0; i + 2 < EditableMeshData.Triangles.Num(); i += 3)
	{
		int Index0 = EditableMeshData.Triangles[i];
		int Index1 = EditableMeshData.Triangles[i + 1];
		int Index2 = EditableMeshData.Triangles[i + 2];

		int V0 = Builder.AddVertex(EditableMeshData.Vertices[Index0])
			.SetNormal(EditableMeshData.Normals[Index0])
			.SetTexCoord(EditableMeshData.UVs[Index0])
			.SetTangent(EditableMeshData.Tangents[Index0])
			.SetColor(USoterioMeshLib::GenerateVertexColor(EditableMeshData.VertexHeat[Index0]));

		int V1 = Builder.AddVertex(EditableMeshData.Vertices[Index1])
			.SetNormal(EditableMeshData.Normals[Index1])
			.SetTexCoord(EditableMeshData.UVs[Index1])
			.SetTangent(EditableMeshData.Tangents[Index1])
			.SetColor(USoterioMeshLib::GenerateVertexColor(EditableMeshData.VertexHeat[Index1]));

		int V2 = Builder.AddVertex(EditableMeshData.Vertices[Index2])
			.SetNormal(EditableMeshData.Normals[Index2])
			.SetTexCoord(EditableMeshData.UVs[Index2])
			.SetTangent(EditableMeshData.Tangents[Index2])
			.SetColor(USoterioMeshLib::GenerateVertexColor(EditableMeshData.VertexHeat[Index2]));

		Builder.AddTriangle(V0, V1, V2, 0);  // Material index set to 0
	}

	USoterioMeshLib::GenerateSpline(*ProductQuery[0], *ProductComponent);
	ProductQuery[0]->Spline->UpdateSpline();

	// Update the realtime mesh with the new builder data
	const FRealtimeMeshSectionGroupKey GroupKey = FRealtimeMeshSectionGroupKey::Create(0, FName("TestTriangle"));
	const FRealtimeMeshSectionKey PolyGroup0SectionKey = FRealtimeMeshSectionKey::CreateForPolyGroup(GroupKey, 0);

	RealtimeMesh->CreateSectionGroup(GroupKey, StreamSet);
	RealtimeMesh->UpdateSectionConfig(PolyGroup0SectionKey, FRealtimeMeshSectionConfig(0), true);
}

FHitResult ABladesmithController::PerformRaycastFromAnvilCamera()
{
	if (!AnvilCamera)
	{
		return FHitResult();
	}

	FVector WorldLocation, WorldDirection;
	Player->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

	FVector StartLocation = AnvilCamera->GetComponentByClass<UCameraComponent>()->GetComponentLocation();
	float TraceDistance = 200.0f;
	FVector EndLocation = WorldLocation + (WorldDirection * TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECollisionChannel::ECC_Camera,
		CollisionParams
	);

	FColor LineColor = bHit ? FColor::Green : FColor::Red;
	float LineLifetime = 1.0f; 
	DrawDebugPoint(
		GetWorld(),
		HitResult.ImpactPoint,
		10.0f,              // Point size
		FColor::Blue,       // Color of the point
		false,              // Persistent (set to true to keep it there indefinitely)
		1.0f                // Duration (set to -1 for infinite)
	);

	if (bHit)
	{
		if (DecalMaterial)
		{
			UGameplayStatics::SpawnDecalAtLocation(
				GetWorld(),
				DecalMaterial,
				FVector(0.2f, 0.2f, 0.2f),
				HitResult.ImpactPoint,
				HitResult.ImpactNormal.Rotation(),
				5.0f 
			);
		}
	}
	return HitResult;

}

void ABladesmithController::SaveGameProgress()
{
	FString JSONString;
	FJsonObjectConverter::UStructToJsonObjectString(GameProgress, JSONString);

	FString SavePath = FPaths::ProjectSavedDir() + GameProgress.SaveName.ToString();

	if (FFileHelper::SaveStringToFile(JSONString, *SavePath))
	{
		UE_LOG(LogBladesmithController, Log, TEXT("Game save successful at %s"), *SavePath);
	}
	else
	{
		UE_LOG(LogBladesmithController, Warning, TEXT("Failed to save game data!"));
	}
}

void ABladesmithController::LoadGameProgress(FString& SaveName)
{
	FString LoadPath = FPaths::ProjectSavedDir() + SaveName;
	FString LoadedJSONString;
	if (FFileHelper::LoadFileToString(LoadedJSONString, *LoadPath))
	{
		if (FJsonObjectConverter::JsonObjectStringToUStruct(LoadedJSONString, &GameProgress, 0, 0))
		{
			UE_LOG(LogBladesmithController, Log, TEXT("Game load successful"));
		}
		else
		{
			UE_LOG(LogBladesmithController, Warning, TEXT("Failed to parse game save data!"));
		}
	}
	else
	{
		UE_LOG(LogBladesmithController, Log, TEXT("No save file found, starting new game."));
		this->GameProgress = FGameProgress();
	}

}


