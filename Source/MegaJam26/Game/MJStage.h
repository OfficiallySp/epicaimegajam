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
class UCameraComponent;
class USpotLightComponent;
class UPointLightComponent;
class USkyLightComponent;
class UDirectionalLightComponent;
class UTextureCube;

USTRUCT()
struct FMJFigure
{
	GENERATED_BODY()

	UPROPERTY() TObjectPtr<USceneComponent> Pivot;  // tile position, hop, yaw
	UPROPERTY() TObjectPtr<USceneComponent> Tilt;   // topple, squash
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Alert;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Carry;
	UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> Mats;
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
	UPROPERTY() TObjectPtr<USceneComponent> Moving; // the part that animates (door slab, lever stick, plate...)
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

	/** Lights, exposure and grading for a reel. */
	UFUNCTION(BlueprintCallable, Category = "MegaJam") void ApplyLook(int32 Reel);

	// Exposure is fixed at EV100 0 so that an emissive value of 1 reads as "bright"; the lights are scaled to match.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float ExposureEV = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float KeyCandelas = 1300.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float FillCandelas = 160.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float SkyIntensity = 1.f;

	FVector TileToWorld(FIntPoint P, float Z = 0.f) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float TileSize = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float WallHeight = 55.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float HopHeight = 38.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float CamFov = 28.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float CamPitch = -52.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float CamLerpSpeed = 3.5f;
	/** Fraction of the screen height the timeline panel covers; the board is framed in what is left. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float PanelFraction = 0.36f;

	UPROPERTY(VisibleAnywhere, Category = "MegaJam") TObjectPtr<USceneComponent> Root;
	UPROPERTY(VisibleAnywhere, Category = "MegaJam") TObjectPtr<UCameraComponent> Camera;
	UPROPERTY(VisibleAnywhere, Category = "MegaJam") TObjectPtr<USpotLightComponent> KeyLight;
	UPROPERTY(VisibleAnywhere, Category = "MegaJam") TObjectPtr<UPointLightComponent> FillLight;
	UPROPERTY(VisibleAnywhere, Category = "MegaJam") TObjectPtr<USkyLightComponent> SkyLight;
	UPROPERTY(VisibleAnywhere, Category = "MegaJam") TObjectPtr<UStaticMeshComponent> Desk;

private:
	UStaticMeshComponent* AddMesh(USceneComponent* Parent, UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale, UMaterialInterface* Mat, const FRotator& Rot = FRotator::ZeroRotator, bool bShadow = true);
	USceneComponent* AddPivot(USceneComponent* Parent, const FVector& Loc);
	UMaterialInstanceDynamic* Solid(const FLinearColor& Color, float Rough = 0.6f, float Glow = 0.f, float Metal = 0.f);
	UMaterialInstanceDynamic* GlowMat(const FLinearColor& Color, float Opacity);
	void BuildFigure(const FMJActorDef& D, FMJFigure& Out);
	void ComputeCamera(EMJCamMode Mode, FVector& OutLoc, FRotator& OutRot) const;
	void ClearDynamic();

	UPROPERTY() TObjectPtr<UStaticMesh> CubeMesh;
	UPROPERTY() TObjectPtr<UStaticMesh> CylMesh;
	UPROPERTY() TObjectPtr<UStaticMesh> ConeMesh;
	UPROPERTY() TObjectPtr<UStaticMesh> SphereMesh;
	UPROPERTY() TObjectPtr<UStaticMesh> PlaneMesh;
	UPROPERTY() TObjectPtr<UMaterialInterface> SolidBase;
	UPROPERTY() TObjectPtr<UMaterialInterface> GlowBase;
	UPROPERTY() TObjectPtr<UTextureCube> SkyCube;

	UPROPERTY() TObjectPtr<USceneComponent> SetRoot; // parent of everything built per level
	UPROPERTY() TArray<TObjectPtr<USceneComponent>> Dynamic;
	UPROPERTY() TArray<FMJFigure> Figures;
	UPROPERTY() TArray<FMJFixtureVis> Fixtures;
	UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> SightPool;
	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> SightMat;
	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> SightAlertMat;

	int32 BoardW = 1, BoardH = 1;
	EMJCamMode CamMode = EMJCamMode::Edit;
	FVector CamLoc = FVector::ZeroVector;
	FRotator CamRot = FRotator::ZeroRotator;
	bool bCamInit = false;
	float CamTime = 0.f;
};
