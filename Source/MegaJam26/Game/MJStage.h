// Fix It In Post - the miniature set: builds the board from a level definition and poses it for any (fractional) beat.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sim/MJTypes.h"
#include "MJStage.generated.h"

class UStaticMesh;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;
class UInstancedStaticMeshComponent;
class UMeshComponent;
class UCameraComponent;
class USpotLightComponent;
class UPointLightComponent;
class USkyLightComponent;
class UTextureCube;

USTRUCT()
struct FMJFigure
{
	GENERATED_BODY()

	UPROPERTY() TObjectPtr<USceneComponent> Pivot;   // tile position, hop, yaw
	UPROPERTY() TObjectPtr<USceneComponent> Tilt;    // topple, squash
	UPROPERTY() TObjectPtr<USceneComponent> Spinner; // knock-out stars
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Alert;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Carry;
	bool bOnSet = true;
};

USTRUCT()
struct FMJFixtureVis
{
	GENERATED_BODY()

	FIntPoint Tile = FIntPoint(0, 0);
	EMJFix Fix = EMJFix::None;
	int32 Chan = 0;
	UPROPERTY() TObjectPtr<USceneComponent> Root;
	UPROPERTY() TObjectPtr<USceneComponent> Moving; // the part that animates (gate block, lever arm, plate top...)
	UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Beams;
};

UENUM()
enum class EMJCamMode : uint8 { Edit, Screening, Title };

UCLASS()
class MEGAJAM26_API AMJStage : public AActor
{
	GENERATED_BODY()

public:
	AMJStage();

	virtual void Tick(float DeltaSeconds) override;

	/** Throw away the current set and build the one for this level. */
	void Build(const FMJLevelDef& L);

	/** Pose everything for a fractional beat of a simulation result. Pure function of its inputs, so it can be scrubbed both ways. */
	void Apply(const FMJLevelDef& L, const FMJSimResult& R, float Time);

	void SetCamMode(EMJCamMode Mode, bool bSnap = false);

	/** Screening coverage: 0 = the wide master, 1 = close on the star, 3 = high three-quarter. A change of shot is a hard cut. */
	void SetShot(int32 InShot);

	/** A short camera shake for impacts (punches, lasers, the car). */
	void Kick(float Strength) { Shake = FMath::Max(Shake, Strength); }

	/** Lights, exposure and grading for a reel. */
	UFUNCTION(BlueprintCallable, Category = "MegaJam") void ApplyLook(int32 Reel);

	FVector TileToWorld(FIntPoint P, float Z = 0.f) const;

	// Exposure is fixed at EV100 0 so that an emissive value of 1 reads as "bright"; the lights are scaled to match.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float ExposureEV = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float KeyCandelas = 1500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float FillCandelas = 170.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float LampCandelas = 260.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float SkyIntensity = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float TileSize = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float HopHeight = 38.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float FigureHeight = 108.f;
	/** Yaw that turns an imported figurine so its face points along +X (the engine's forward). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float FigureYaw = 90.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float CarYaw = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float LampYaw = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float FlatHeightScale = 0.85f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float CamFov = 28.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float CamPitch = -52.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float CamLerpSpeed = 3.5f;
	/** Tilt-shift: a wide virtual sensor gives a shallow depth of field, which is what makes the set read as a miniature. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float DofSensorWidth = 900.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float DofFstop = 2.6f;
	/** Fraction of the screen height the timeline panel covers; the board is framed in what is left. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float PanelFraction = 0.36f;

	UPROPERTY(VisibleAnywhere, Category = "MegaJam") TObjectPtr<USceneComponent> Root;
	UPROPERTY(VisibleAnywhere, Category = "MegaJam") TObjectPtr<UCameraComponent> Camera;
	UPROPERTY(VisibleAnywhere, Category = "MegaJam") TObjectPtr<USpotLightComponent> KeyLight;
	UPROPERTY(VisibleAnywhere, Category = "MegaJam") TObjectPtr<UPointLightComponent> FillLight;
	UPROPERTY(VisibleAnywhere, Category = "MegaJam") TObjectPtr<UPointLightComponent> LampLight;
	UPROPERTY(VisibleAnywhere, Category = "MegaJam") TObjectPtr<USkyLightComponent> SkyLight;

private:
	struct FSlotMat { const TCHAR* Slot; UMaterialInterface* Mat; };

	UStaticMesh* Kit(const TCHAR* Name);
	UStaticMesh* LoadMesh(const FString& ObjectPath);
	void ApplySlots(UMeshComponent* Comp, UStaticMesh* Mesh, std::initializer_list<FSlotMat> Slots) const;
	UStaticMeshComponent* AddMesh(USceneComponent* Parent, UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale, UMaterialInterface* Mat, const FRotator& Rot = FRotator::ZeroRotator, bool bShadow = true);
	UStaticMeshComponent* AddKit(USceneComponent* Parent, const TCHAR* Name, const FVector& Loc, std::initializer_list<FSlotMat> Slots, const FRotator& Rot = FRotator::ZeroRotator, const FVector& Scale = FVector(1.f), bool bShadow = true);
	UInstancedStaticMeshComponent* AddInstanced(const TCHAR* Name, std::initializer_list<FSlotMat> Slots);
	USceneComponent* AddPivot(USceneComponent* Parent, const FVector& Loc);
	/** A desk prop (generated mesh of any size and pivot): scaled so its longest side is LongestSide, standing on Z. */
	void AddProp(const TCHAR* Name, const FVector& Loc, float Yaw, float LongestSide);
	UMaterialInstanceDynamic* Solid(const FLinearColor& Color, float Rough = 0.6f, float Glow = 0.f, float Metal = 0.f);
	UMaterialInstanceDynamic* GlowMat(const FLinearColor& Color, float Opacity);
	void BuildFigure(const FMJActorDef& D, FMJFigure& Out);
	void ComputeCamera(EMJCamMode Mode, FVector& OutLoc, FRotator& OutRot, float& OutFocus) const;
	void ClearDynamic();

	UPROPERTY() TObjectPtr<UStaticMesh> CubeMesh;
	UPROPERTY() TObjectPtr<UStaticMesh> CylMesh;
	UPROPERTY() TObjectPtr<UStaticMesh> ConeMesh;
	UPROPERTY() TObjectPtr<UStaticMesh> SphereMesh;
	UPROPERTY() TObjectPtr<UMaterialInterface> SolidBase;
	UPROPERTY() TObjectPtr<UMaterialInterface> GlowBase;
	UPROPERTY() TObjectPtr<UTextureCube> SkyCube;
	UPROPERTY() TMap<FName, TObjectPtr<UStaticMesh>> MeshCache;

	UPROPERTY() TObjectPtr<USceneComponent> SetRoot; // parent of everything built per level
	UPROPERTY() TArray<TObjectPtr<USceneComponent>> Dynamic;
	UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> Mats; // keeps this level's materials alive
	UPROPERTY() TArray<FMJFigure> Figures;
	UPROPERTY() TArray<FMJFixtureVis> Fixtures;
	UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> SightPool;
	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> SightEdge;
	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> SightFill;
	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> AlertEdge;
	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> AlertFill;

	int32 BoardW = 1, BoardH = 1;
	EMJCamMode CamMode = EMJCamMode::Edit;
	FVector CamLoc = FVector::ZeroVector;
	FRotator CamRot = FRotator::ZeroRotator;
	float CamFocus = 3000.f;
	bool bCamInit = false;
	int32 Shot = 0;
	bool bShotCut = false;
	int32 StarFigure = 0;
	float Shake = 0.f;
	float CamTime = 0.f;
};
