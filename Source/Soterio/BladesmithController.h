// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Camera/CameraComponent.h"
#include "SoterioMeshLib.h"
#include "EngineUtils.h"
#include "Logging/LogMacros.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/EngineTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

#include "../../Plugins/RealtimeMeshComponent/Source/RealtimeMeshComponent/Public/RealtimeMeshComponent.h"
#include "../../Plugins/RealtimeMeshComponent/Source/RealtimeMeshComponent/Public/RealtimeMeshSimple.h"
#include "../../Plugins/RealtimeMeshComponent/Source/RealtimeMeshComponent/Public/RealtimeMeshCollisionLibrary.h"

#include "GameTypes.h"
#include "S_Material.h"
#include "DebugOverlay.h"
#include "SoterioCharacter.h"

#include "BladesmithController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBladesmithController , Log, All);

DECLARE_MULTICAST_DELEGATE(FDayChange)

class ACameraActor;

#define SOCKET_BS_LEFT_HAND "BS_LEFT_HAND"
#define SOCKET_BS_LEFT_SHIT "BS_RIGHT_HAND"
#define SOCKET_ANVIL "AnvilSocket"

UCLASS()
class SOTERIO_API ABladesmithController : public AActor
{
	GENERATED_BODY()

private:
	FDayChange OnDayChange;

	FTimerHandle GameProgressTimeHandle;

	bool bIsCameraActive;

	ES_GameMode CurrentMode;

	ASoterioCharacter* Character;

	APlayerController* Player;

	TArray<FHammerData*> HammerList;

	UDecalComponent* OreIndicator;

	UMaterialInterface* FlatIndicatorMaterial;

	int CurrentHammerIndex;

	float ForgeHeatMultipler;

	FHammerData* CurrentHammer;

	//TArray<URealtimeMeshComponent*> ProductQuery;

	TArray<SB_Material> Inventory;

	float FurnaceHeat;

	UMaterialInstanceDynamic* DynamicMaterial;

	TArray<FProductProperties*> ProductQuery;
public:
	ABladesmithController();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forge");
	float BellowBlowAmount;

	UDebugOverlay* DebugOverlay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float DayDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraBlendTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Camera")
	ACameraActor* AnvilCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Camera")
	ACameraActor* ForgeCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Camera")
	ACameraActor* GrindCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Camera")
	ACameraActor* AnvilModCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Camera")
	ACameraActor* WorkbenchCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anvil")
	UMaterialInstance* DecalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anvil")
	int DefaultSmoothRate = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* HammerData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bConsoleDebug_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bScreenDebug;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* BaseStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* ProductMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameProgress GameProgress;

	AActor* AnvilActor;

	AActor* ForgeActor;

	AActor* GrindActor;

	AActor* WorkbenchActor;

private:
	void Bindings();
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<URealtimeMeshComponent> ProductComponent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void TestController(APlayerController* PC);
	void SwitchHammer(bool bDirection);
	
	void SpawnDecal(FVector3f Location, ES_HAMMER_SHAPE Shape);
	/*
	Logging
	*/
	void LogWarning(FString Text);
	void LogError(FString Text);

	/*
	Game Mode Operations
	*/
	void SwitchGameMode(FHitResult HitResult);
	void SwitchCamera();

	/// <summary>
	/// Init Funct
	/// </summary>
	void InitHammerList();

	UFUNCTION()
	void TimePasses();

	void CreateOreInstance();
	void HeatUpForge();
	void SwitchDefaultMode();

	void UpdateProduct(int CalculateNormalsDepth = 0);
	FHitResult PerformRaycastFromAnvilCamera();

	void SaveGameProgress();
	void LoadGameProgress(FString& SaveName);
};
