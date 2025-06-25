// GameTypes.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "S_Material.h"
#include "Components/SplineComponent.h"
#include "GameTypes.generated.h"


// Enums
UENUM(BlueprintType)
enum class ES_GameMode : uint8
{
    Default,
    Anvil,
    AnvilMode,
    Furnace,
    Forge,
    Grindstone,
    Workbench,
    Dialog
};

UENUM()
enum class ES_Material : uint8
{
    Copper  UMETA(DisplayName = "Copper"),
    Bronze  UMETA(DisplayName = "Bronze"),
    Metal   UMETA(DisplayName = "Metal"),
    Gold    UMETA(DisplayName = "Gold")
};

UENUM(BlueprintType)
enum class EQuality : uint8
{
    HIGH UMETA(DisplayName = "High Quality"),
    MEH UMETA(DisplayName = "Meh Quality"),
    LOW UMETA(DisplayName = "Low Quality")
};

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    STORY UMETA(DisplayName = "Story Quest"),
    REGULAR UMETA(DisplayName = "Regular Quest")
};

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
    NOT_STARTED UMETA(DisplayName = "Not Started"),
    ACTIVE      UMETA(DisplayName = "Active"),
    SUCCEED     UMETA(DisplayName = "Success"),
    FAILED      UMETA(DisplayName = "Failed"),
    ON_HOLD     UMETA(DisplayName = "On Hold (waiting for response)"),
    LOCKED      UMETA(DisplayName = "Haven't Unlocked Yet")
};

UENUM(BlueprintType)
enum class ESwordType : uint8
{
    None = 0 UMETA(DisplayName = "None"),
    Dagger = 30 UMETA(DisplayName = "Dagger"),
    ShortSword = 65 UMETA(DisplayName = "Short Sword"),
    LongSword  = 100 UMETA(DisplayName = "Long Sword"),
    GreatSword  = 130 UMETA(DisplayName = "Great Sword")
};


// Structs
USTRUCT(BlueprintType)
struct FQuestProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ES_Material SwordMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESwordType SwordType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Hardness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Count;
};

struct FEMMaterial
{
    float YoungModulus; // (E)
    float PoissonRatio; // (v)

    FEMMaterial(float E, float nu)
        : YoungModulus(E), PoissonRatio(nu) {}
};

struct FEMNode
{
    FVector Position;
    bool bIsFixed;
    FVector Displacement;

    FEMNode(FVector position, bool isFixed)
        : Position(position), bIsFixed(isFixed), Displacement(FVector(0, 0, 0)) {}
};


USTRUCT(BlueprintType)
struct FQuest : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestStatus QuestStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuality AcceptableQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuestProperties> Properties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float Time;
};

USTRUCT(BlueprintType)
struct FProductProperties
{
    GENERATED_BODY()

    UPROPERTY()
    FName ProductName;

    UPROPERTY()
    bool bIsMaxLength;

    UPROPERTY()
    ESwordType Type;

    UPROPERTY()
    FGuid ProductID;

    UPROPERTY()
    TArray<FVector3f> Vertices;

    UPROPERTY()
    TArray<int32> Triangles;

    UPROPERTY()
    TArray<FVector3f> Normals;

    UPROPERTY()
    TArray<FVector2f> UVs;

    UPROPERTY()
    TArray<FVector3f> Tangents;

    UPROPERTY()
    TArray<float> VertexHeat;

    UPROPERTY()
    TArray<FVector> SplinePoints;

    UPROPERTY()
    float Length;

    UPROPERTY()
    float MaxLength;

    UPROPERTY()
	float MaxThickness;

    float SplineDisp;

    UPROPERTY()
    TObjectPtr<USplineComponent> Spline;

    FProductProperties() {}

    FProductProperties(
        const TArray<FVector3f>& vert,
        const TArray<int32>& tri,
        const TArray<FVector3f>& norm,
        const TArray<FVector2f>& uv,
        const TArray<FVector3f>& tang
    )
        : Vertices(vert), Triangles(tri), Normals(norm), UVs(uv), Tangents(tang)
    {
        Spline = NULL;
        ProductID = FGuid::NewGuid();
        MaxLength = 30;
		MaxThickness = 0.1;
        bIsMaxLength = false;
        UE_LOG(LogTemp, Warning, TEXT("%s is the GUID"), *ProductID.ToString());
    }

    void Serialize(FArchive& Ar)
    {
        Ar << ProductID;
        Ar << Vertices;
        Ar << Triangles;
        Ar << Normals;
        Ar << UVs;
        Ar << Tangents;
        Ar << VertexHeat;
        Ar << SplinePoints;
    }

    uint8 _Debug_averageHeat()
    {
        uint8 sum = 0;
        for (int i = 0; i < VertexHeat.Num(); i++)
        {
            sum += VertexHeat[i];
        }
        return sum /= VertexHeat.Num();
    }

    void GenerateSplineData()
    {
        if (Vertices.IsEmpty())
        {
            return;
        }

        FVector start(0, FLT_MAX, 0);
        FVector end(0, -FLT_MAX, 0);

        for (const FVector3f& Vert : Vertices)
        {
            if (Vert.Y < start.Y)
            {
                start.Y = Vert.Y;
            }
            if (Vert.Y > end.Y)
            {
                end.Y = Vert.Y;
            }
        }

        MaxLength = 65;
        float count = 8;
        Length = FVector::Distance(start, end);
        SplinePoints.Empty();
        SplinePoints.Add(start);
        float splineDisp = (Length/ count);
        for (int i = 1; i < count - 1; i++)
        {
            SplinePoints.Add(FVector(0, (SplinePoints[i - 1].Y + SplineDisp), 0));
        }

        SplinePoints.Add(end);

        if (Length >= MaxLength)
        {
            bIsMaxLength = true;
        }
        else
        {
            bIsMaxLength = false;
        }

        //UE_LOG(LogTemp, Warning, TEXT("Start: %s, End: %s, SplinePoints: %d Length: %f, MaxLength: %f, bIsMaxLength: %s"),
        //    *start.ToString(), *end.ToString(), SplinePoints.Num(), Length, MaxLength, bIsMaxLength ? TEXT("true") : TEXT("false"));
    }


};

UENUM(BlueprintType)
enum class ES_Months : uint8
{
    January,
    February,
    March
};

UENUM(BlueprintType)
enum class ES_HAMMER_SHAPE : uint8
{
    FLAT UMETA(DisplayName = "Flat"),
    ROUND UMETA(DisplayName = "Rounded"),
    SHARP UMETA(DisplayName = "Sharp")
};


UENUM(BlueprintType)
enum class ES_DaysOfWeek : uint8
{
    Sunday UMETA(DisplayName = "Sunday"),
    Monday UMETA(DisplayName = "Monday"),
    Tuesday UMETA(DisplayName = "Tuesday"),
    Wednesday UMETA(DisplayName = "Wednesday"),
    Thursday UMETA(DisplayName = "Thursday"),
    Friday UMETA(DisplayName = "Friday"),
    Saturday UMETA(DisplayName = "Saturday")
};


USTRUCT(BlueprintType)
struct FHammerData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Weigth;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Size;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ES_HAMMER_SHAPE Face_0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ES_HAMMER_SHAPE Face_1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* Mesh;
};
USTRUCT(BlueprintType)
struct FGameDate
{
    GENERATED_BODY()

    UPROPERTY()
    uint8 TimeOfDay;

    UPROPERTY()
    ES_DaysOfWeek Day;

    UPROPERTY()
    ES_Months Month;
};

USTRUCT(BlueprintType)
struct FGameProperties
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float Barta = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float Iron = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float Coal = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float Gold = 0.0f;

    //UPROPERTY()
    //TArray<S_Material> PlayerInventory;

    UPROPERTY(BlueprintReadWrite)
    TArray<FQuest> CompletedQuests;

    UPROPERTY(BlueprintReadWrite)
    TArray<FGuid> Products;

    FGameProperties() {}
};


USTRUCT(BlueprintType)
struct FGameProgress
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FName SaveName;
    
    UPROPERTY(BlueprintReadWrite)
    FGameProperties GameProperties;

    UPROPERTY(BlueprintReadWrite)
    FGameDate Date;

    FGameProgress()
        : SaveName("DefaultSave"),
        GameProperties()
    {}
};

class GameTypes
{
public:
    GameTypes();
    ~GameTypes();
};
