// Fix It In Post - the miniature set.
#include "Game/MJStage.h"

#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureCube.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** Load an optional asset by object path ("/Game/Dir/Name.Name"); missing assets are normal (primitive fallbacks). */
	template <class T>
	T* LoadQuiet(const TCHAR* Path)
	{
		const FString Package = FPackageName::ObjectPathToPackageName(FString(Path));
		if (!FPackageName::DoesPackageExist(Package)) return nullptr;
		return Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, Path, nullptr, LOAD_NoWarn | LOAD_Quiet));
	}

	FORCEINLINE float Ease(float X)
	{
		X = FMath::Clamp(X, 0.f, 1.f);
		return X * X * (3.f - 2.f * X);
	}

	FORCEINLINE float DirYaw(EMJDir D)
	{
		switch (D)
		{
		case EMJDir::N: return 0.f;
		case EMJDir::E: return 90.f;
		case EMJDir::S: return 180.f;
		case EMJDir::W: return 270.f;
		default: return 0.f;
		}
	}

	FLinearColor ChanColor(int32 Chan)
	{
		static const FLinearColor Colors[4] = {
			FLinearColor(1.0f, 0.55f, 0.08f), FLinearColor(0.08f, 0.75f, 0.9f), FLinearColor(0.9f, 0.15f, 0.65f), FLinearColor(0.45f, 0.9f, 0.15f) };
		return Colors[FMath::Clamp(Chan, 0, 3)];
	}
}

AMJStage::AMJStage()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeF(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylF(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeF(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereF(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneF(TEXT("/Engine/BasicShapes/Plane.Plane"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMatF(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	static ConstructorHelpers::FObjectFinder<UTextureCube> SkyF(TEXT("/Engine/EngineResources/GrayDarkTextureCube.GrayDarkTextureCube"));
	CubeMesh = CubeF.Object;
	CylMesh = CylF.Object;
	ConeMesh = ConeF.Object;
	SphereMesh = SphereF.Object;
	PlaneMesh = PlaneF.Object;
	SolidBase = BasicMatF.Object;
	GlowBase = BasicMatF.Object;
	SkyCube = SkyF.Object;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Root);
	Camera->SetFieldOfView(CamFov);
	Camera->bConstrainAspectRatio = false;
	Camera->PostProcessBlendWeight = 1.f;

	KeyLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("KeyLight"));
	KeyLight->SetupAttachment(Root);
	KeyLight->SetMobility(EComponentMobility::Movable);
	KeyLight->IntensityUnits = ELightUnits::Candelas;

	FillLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FillLight"));
	FillLight->SetupAttachment(Root);
	FillLight->SetMobility(EComponentMobility::Movable);
	FillLight->IntensityUnits = ELightUnits::Candelas;

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(Root);
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->SourceType = ESkyLightSourceType::SLS_SpecifiedCubemap;
	SkyLight->Cubemap = SkyCube;

	Desk = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Desk"));
	Desk->SetupAttachment(Root);
	Desk->SetStaticMesh(CubeMesh);
	Desk->SetRelativeLocation(FVector(0.f, 0.f, -70.f));
	Desk->SetRelativeScale3D(FVector(80.f, 80.f, 1.f));
	Desk->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMJStage::ApplyLook(int32 Reel)
{
	// One accent colour per reel; everything else is a tungsten desk lamp in a dark cutting room.
	static const FLinearColor Accents[3] = { FLinearColor(0.10f, 0.55f, 1.0f), FLinearColor(1.0f, 0.15f, 0.65f), FLinearColor(1.0f, 0.16f, 0.08f) };
	const FLinearColor Accent = Accents[FMath::Clamp(Reel - 1, 0, 2)];

	FPostProcessSettings& PP = Camera->PostProcessSettings;
	PP.bOverride_AutoExposureMethod = true; PP.AutoExposureMethod = AEM_Manual;
	PP.bOverride_AutoExposureBias = true; PP.AutoExposureBias = -ExposureEV; // manual exposure sits at EV100 0, so the bias is the whole exposure
	PP.bOverride_AutoExposureApplyPhysicalCameraExposure = true; PP.AutoExposureApplyPhysicalCameraExposure = false;
	PP.bOverride_VignetteIntensity = true; PP.VignetteIntensity = 0.5f;
	PP.bOverride_MotionBlurAmount = true; PP.MotionBlurAmount = 0.f;
	PP.bOverride_BloomIntensity = true; PP.BloomIntensity = 0.8f;
	PP.bOverride_FilmGrainIntensity = true; PP.FilmGrainIntensity = 0.25f;

	KeyLight->SetRelativeLocation(FVector(-700.f, -1000.f, 1700.f));
	KeyLight->SetIntensityUnits(ELightUnits::Candelas);
	KeyLight->SetIntensity(KeyCandelas);
	KeyLight->SetAttenuationRadius(8000.f);
	KeyLight->SetInnerConeAngle(20.f);
	KeyLight->SetOuterConeAngle(40.f);
	KeyLight->SetSourceRadius(45.f);
	KeyLight->SetLightColor(FLinearColor(1.0f, 0.80f, 0.58f));
	KeyLight->SetCastShadows(true);
	KeyLight->SetRelativeRotation((FVector(0.f, 0.f, 0.f) - KeyLight->GetRelativeLocation()).Rotation());

	FillLight->SetRelativeLocation(FVector(-1100.f, 1500.f, 800.f));
	FillLight->SetIntensityUnits(ELightUnits::Candelas);
	FillLight->SetIntensity(FillCandelas);
	FillLight->SetAttenuationRadius(8000.f);
	FillLight->SetSourceRadius(150.f);
	FillLight->SetLightColor(Accent);
	FillLight->SetCastShadows(false);

	SkyLight->SetIntensity(SkyIntensity);
	SkyLight->SetLightColor(FLinearColor(0.45f, 0.55f, 1.0f));
}

FVector AMJStage::TileToWorld(FIntPoint P, float Z) const
{
	return FVector(-(P.Y - (BoardH - 1) * 0.5f) * TileSize, (P.X - (BoardW - 1) * 0.5f) * TileSize, Z);
}

UStaticMeshComponent* AMJStage::AddMesh(USceneComponent* Parent, UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale, UMaterialInterface* Mat, const FRotator& Rot, bool bShadow)
{
	UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
	C->SetMobility(EComponentMobility::Movable);
	C->SetStaticMesh(Mesh);
	C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	C->SetCastShadow(bShadow);
	C->SetupAttachment(Parent ? Parent : SetRoot.Get());
	C->SetRelativeLocationAndRotation(Loc, Rot);
	C->SetRelativeScale3D(Scale);
	if (Mat) C->SetMaterial(0, Mat);
	C->RegisterComponent();
	Dynamic.Add(C);
	return C;
}

USceneComponent* AMJStage::AddPivot(USceneComponent* Parent, const FVector& Loc)
{
	USceneComponent* C = NewObject<USceneComponent>(this);
	C->SetMobility(EComponentMobility::Movable);
	C->SetupAttachment(Parent ? Parent : SetRoot.Get());
	C->SetRelativeLocation(Loc);
	C->RegisterComponent();
	Dynamic.Add(C);
	return C;
}

UMaterialInstanceDynamic* AMJStage::Solid(const FLinearColor& Color, float Rough, float Glow, float Metal)
{
	UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(SolidBase, this);
	M->SetVectorParameterValue(TEXT("Color"), Color);
	M->SetScalarParameterValue(TEXT("Rough"), Rough);
	M->SetScalarParameterValue(TEXT("Glow"), Glow);
	M->SetScalarParameterValue(TEXT("Metal"), Metal);
	return M;
}

UMaterialInstanceDynamic* AMJStage::GlowMat(const FLinearColor& Color, float Opacity)
{
	UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(GlowBase, this);
	M->SetVectorParameterValue(TEXT("Color"), Color);
	M->SetScalarParameterValue(TEXT("Opacity"), Opacity);
	return M;
}

void AMJStage::ClearDynamic()
{
	for (USceneComponent* C : Dynamic) { if (C) C->DestroyComponent(); }
	Dynamic.Reset();
	Figures.Reset();
	Fixtures.Reset();
	SightPool.Reset();
	if (SetRoot) { SetRoot->DestroyComponent(); SetRoot = nullptr; }
}

void AMJStage::BuildFigure(const FMJActorDef& D, FMJFigure& Out)
{
	Out.bOnSet = D.Role != EMJRole::Prop && D.Start.X >= 0;
	Out.Pivot = AddPivot(nullptr, TileToWorld(D.Start));
	Out.Tilt = AddPivot(Out.Pivot, FVector::ZeroVector);
	if (!Out.bOnSet) { Out.Pivot->SetVisibility(false, true); return; }

	FLinearColor Body(0.8f, 0.8f, 0.8f), Base(0.08f, 0.08f, 0.09f), Accent(1.f, 1.f, 1.f);
	switch (D.Role)
	{
	case EMJRole::Star: Body = FLinearColor(0.92f, 0.9f, 0.84f); Accent = FLinearColor(1.0f, 0.72f, 0.1f); Base = FLinearColor(0.75f, 0.52f, 0.08f); break;
	case EMJRole::Ally: Body = FLinearColor(0.1f, 0.55f, 0.6f); Accent = FLinearColor(0.95f, 0.85f, 0.7f); Base = FLinearColor(0.05f, 0.3f, 0.34f); break;
	case EMJRole::Guard: Body = FLinearColor(0.12f, 0.14f, 0.32f); Accent = FLinearColor(0.85f, 0.2f, 0.15f); Base = FLinearColor(0.35f, 0.06f, 0.05f); break;
	case EMJRole::Villain: Body = FLinearColor(0.55f, 0.04f, 0.08f); Accent = FLinearColor(0.9f, 0.75f, 0.65f); Base = FLinearColor(0.2f, 0.02f, 0.03f); break;
	case EMJRole::Camera: Body = FLinearColor(0.3f, 0.3f, 0.33f); Accent = FLinearColor(0.9f, 0.1f, 0.1f); Base = FLinearColor(0.1f, 0.1f, 0.1f); break;
	case EMJRole::Crusher: Body = FLinearColor(0.75f, 0.1f, 0.08f); Accent = FLinearColor(0.95f, 0.9f, 0.6f); Base = FLinearColor(0.05f, 0.05f, 0.05f); break;
	default: break;
	}

	// A dedicated figurine mesh wins over the primitive stand-in.
	const FString MeshPath = FString::Printf(TEXT("/Game/MegaJam26/Characters/SM_%s.SM_%s"), *D.Look.ToString(), *D.Look.ToString());
	if (UStaticMesh* Custom = LoadQuiet<UStaticMesh>(*MeshPath))
	{
		// Generated figurines arrive at any size and pivot: stand them on the tile, scaled to a common height.
		const FBoxSphereBounds B = Custom->GetBounds();
		const float Scale = FigureHeight / FMath::Max(1.f, B.BoxExtent.Z * 2.f);
		const FRotator Rot(0.f, FigureYaw, 0.f);
		const FVector Foot = B.Origin - FVector(0.f, 0.f, B.BoxExtent.Z);
		AddMesh(Out.Tilt, Custom, -Rot.RotateVector(Foot * Scale), FVector(Scale), nullptr, Rot);
	}
	else if (D.Role == EMJRole::Crusher)
	{
		AddMesh(Out.Tilt, CubeMesh, FVector(0.f, 0.f, 28.f), FVector(0.86f, 0.6f, 0.36f), Solid(Body, 0.3f));
		AddMesh(Out.Tilt, CubeMesh, FVector(-6.f, 0.f, 58.f), FVector(0.44f, 0.5f, 0.26f), Solid(Accent, 0.2f));
	}
	else if (D.Role == EMJRole::Camera)
	{
		AddMesh(Out.Tilt, CylMesh, FVector(0.f, 0.f, 45.f), FVector(0.08f, 0.08f, 0.9f), Solid(Base, 0.4f));
		AddMesh(Out.Tilt, CubeMesh, FVector(8.f, 0.f, 95.f), FVector(0.42f, 0.22f, 0.2f), Solid(Body, 0.4f));
		AddMesh(Out.Tilt, SphereMesh, FVector(30.f, 0.f, 95.f), FVector(0.1f), Solid(Accent, 0.3f, 6.f));
	}
	else
	{
		AddMesh(Out.Tilt, CylMesh, FVector(0.f, 0.f, 4.f), FVector(0.66f, 0.66f, 0.08f), Solid(Base, 0.35f));
		AddMesh(Out.Tilt, ConeMesh, FVector(0.f, 0.f, 44.f), FVector(0.5f, 0.5f, 0.72f), Solid(Body, 0.55f));
		AddMesh(Out.Tilt, SphereMesh, FVector(0.f, 0.f, 88.f), FVector(0.3f), Solid(Accent, 0.5f));
		AddMesh(Out.Tilt, CubeMesh, FVector(15.f, 0.f, 90.f), FVector(0.1f, 0.2f, 0.06f), Solid(FLinearColor(0.02f, 0.02f, 0.02f), 0.3f)); // visor: shows facing
	}

	Out.Alert = AddMesh(Out.Pivot, ConeMesh, FVector(0.f, 0.f, 150.f), FVector(0.22f, 0.22f, 0.4f), Solid(FLinearColor(1.f, 0.1f, 0.05f), 0.4f, 12.f), FRotator(180.f, 0.f, 0.f), false);
	Out.Alert->SetVisibility(false);
	Out.Carry = AddMesh(Out.Pivot, SphereMesh, FVector(0.f, 0.f, 128.f), FVector(0.16f), Solid(FLinearColor(0.3f, 0.9f, 1.f), 0.1f, 4.f), FRotator::ZeroRotator, false);
	Out.Carry->SetVisibility(false);
}

void AMJStage::Build(const FMJLevelDef& L)
{
	ClearDynamic();

	if (UMaterialInterface* M = LoadQuiet<UMaterialInterface>(TEXT("/Game/MegaJam26/Materials/M_Solid.M_Solid"))) SolidBase = M;
	if (UMaterialInterface* M = LoadQuiet<UMaterialInterface>(TEXT("/Game/MegaJam26/Materials/M_Glow.M_Glow"))) GlowBase = M;

	BoardW = FMath::Max(1, L.W);
	BoardH = FMath::Max(1, L.H);

	SetRoot = NewObject<USceneComponent>(this);
	SetRoot->SetMobility(EComponentMobility::Movable);
	SetRoot->SetupAttachment(Root);
	SetRoot->RegisterComponent();

	Desk->SetMaterial(0, Solid(FLinearColor(0.16f, 0.09f, 0.05f), 0.45f));

	auto MakeISM = [this](UMaterialInterface* Mat) -> UInstancedStaticMeshComponent*
	{
		UInstancedStaticMeshComponent* C = NewObject<UInstancedStaticMeshComponent>(this);
		C->SetMobility(EComponentMobility::Movable);
		C->SetStaticMesh(CubeMesh);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetMaterial(0, Mat);
		C->SetupAttachment(SetRoot);
		C->RegisterComponent();
		Dynamic.Add(C);
		return C;
	};

	UInstancedStaticMeshComponent* FloorA = MakeISM(Solid(FLinearColor(0.50f, 0.47f, 0.42f), 0.7f));
	UInstancedStaticMeshComponent* FloorB = MakeISM(Solid(FLinearColor(0.40f, 0.37f, 0.33f), 0.7f));
	UInstancedStaticMeshComponent* Walls = MakeISM(Solid(FLinearColor(0.20f, 0.23f, 0.33f), 0.8f));
	UInstancedStaticMeshComponent* Pits = MakeISM(Solid(FLinearColor(0.01f, 0.01f, 0.015f), 0.9f));

	// the plinth the set stands on
	AddMesh(nullptr, CubeMesh, FVector(0.f, 0.f, -40.f), FVector(BoardH + 0.5f, BoardW + 0.5f, 0.5f), Solid(FLinearColor(0.07f, 0.06f, 0.06f), 0.5f));

	const float TS = TileSize / 100.f;
	for (int32 Y = 0; Y < L.H; ++Y)
	{
		for (int32 X = 0; X < L.W; ++X)
		{
			const FIntPoint P(X, Y);
			const FMJCell& C = L.Cell(P);
			if (C.Tile == EMJTile::Void) continue;
			if (C.Tile == EMJTile::Wall)
			{
				Walls->AddInstance(FTransform(FRotator::ZeroRotator, TileToWorld(P, WallHeight * 0.5f - 10.f), FVector(TS, TS, (WallHeight + 20.f) / 100.f)));
				continue;
			}
			if (C.Tile == EMJTile::Pit)
			{
				Pits->AddInstance(FTransform(FRotator::ZeroRotator, TileToWorld(P, -13.f), FVector(TS * 0.98f, TS * 0.98f, 0.1f)));
				continue;
			}
			(((X + Y) & 1) ? FloorA : FloorB)->AddInstance(FTransform(FRotator::ZeroRotator, TileToWorld(P, -5.f), FVector(TS * 0.97f, TS * 0.97f, 0.1f)));

			if (C.Fix == EMJFix::None) continue;
			FMJFixtureVis V;
			V.Tile = P; V.Fix = C.Fix; V.Chan = C.Chan;
			V.Root = AddPivot(nullptr, TileToWorld(P));
			const FLinearColor CC = ChanColor(C.Chan);
			switch (C.Fix)
			{
			case EMJFix::Door:
				V.Moving = AddPivot(V.Root, FVector::ZeroVector);
				AddMesh(V.Moving, CubeMesh, FVector(0.f, 0.f, 42.f), FVector(0.9f, 0.9f, 0.84f), Solid(CC * 0.55f, 0.45f));
				AddMesh(V.Moving, CubeMesh, FVector(0.f, 0.f, 85.f), FVector(0.94f, 0.94f, 0.04f), Solid(CC, 0.3f, 1.5f));
				break;
			case EMJFix::Plate:
				AddMesh(V.Root, CubeMesh, FVector(0.f, 0.f, 1.f), FVector(0.8f, 0.8f, 0.02f), Solid(FLinearColor(0.05f, 0.05f, 0.05f), 0.4f));
				V.Moving = AddPivot(V.Root, FVector::ZeroVector);
				AddMesh(V.Moving, CubeMesh, FVector(0.f, 0.f, 4.f), FVector(0.66f, 0.66f, 0.06f), Solid(CC, 0.35f, 0.6f));
				break;
			case EMJFix::Lever:
				AddMesh(V.Root, CubeMesh, FVector(-30.f, 0.f, 6.f), FVector(0.26f, 0.4f, 0.12f), Solid(FLinearColor(0.06f, 0.06f, 0.07f), 0.4f));
				V.Moving = AddPivot(V.Root, FVector(-30.f, 0.f, 10.f));
				AddMesh(V.Moving, CylMesh, FVector(0.f, 0.f, 26.f), FVector(0.07f, 0.07f, 0.52f), Solid(FLinearColor(0.7f, 0.7f, 0.72f), 0.25f, 0.f, 1.f));
				AddMesh(V.Moving, SphereMesh, FVector(0.f, 0.f, 54.f), FVector(0.17f), Solid(CC, 0.3f, 1.5f));
				break;
			case EMJFix::Laser:
				AddMesh(V.Root, CubeMesh, FVector(0.f, 0.f, 1.5f), FVector(0.9f, 0.9f, 0.03f), Solid(FLinearColor(0.03f, 0.03f, 0.04f), 0.3f));
				for (int32 i = 0; i < 4; ++i)
				{
					const float O = -33.f + i * 22.f;
					V.Beams.Add(AddMesh(V.Root, CylMesh, FVector(O, O * ((i & 1) ? -1.f : 1.f) * 0.6f, 50.f), FVector(0.035f, 0.035f, 1.0f), GlowMat(FLinearColor(1.f, 0.05f, 0.03f), 0.9f), FRotator::ZeroRotator, false));
				}
				break;
			case EMJFix::Item:
				V.Moving = AddPivot(V.Root, FVector(0.f, 0.f, 46.f));
				AddMesh(V.Root, CylMesh, FVector(0.f, 0.f, 14.f), FVector(0.3f, 0.3f, 0.28f), Solid(FLinearColor(0.05f, 0.05f, 0.06f), 0.4f));
				AddMesh(V.Moving, ConeMesh, FVector(0.f, 0.f, 10.f), FVector(0.3f, 0.3f, 0.2f), Solid(FLinearColor(0.3f, 0.9f, 1.f), 0.08f, 2.5f));
				AddMesh(V.Moving, ConeMesh, FVector(0.f, 0.f, -8.f), FVector(0.3f, 0.3f, 0.16f), Solid(FLinearColor(0.3f, 0.9f, 1.f), 0.08f, 2.5f), FRotator(180.f, 0.f, 0.f));
				break;
			case EMJFix::Exit:
				AddMesh(V.Root, CubeMesh, FVector(0.f, 0.f, 1.f), FVector(0.72f, 0.13f, 0.012f), Solid(FLinearColor(1.f, 0.8f, 0.05f), 0.5f, 2.f), FRotator(0.f, 45.f, 0.f), false);
				AddMesh(V.Root, CubeMesh, FVector(0.f, 0.f, 1.f), FVector(0.72f, 0.13f, 0.012f), Solid(FLinearColor(1.f, 0.8f, 0.05f), 0.5f, 2.f), FRotator(0.f, -45.f, 0.f), false);
				break;
			case EMJFix::Bonus:
				V.Moving = AddPivot(V.Root, FVector(0.f, 0.f, 40.f));
				AddMesh(V.Moving, CylMesh, FVector::ZeroVector, FVector(0.42f, 0.42f, 0.07f), Solid(FLinearColor(1.f, 0.75f, 0.1f), 0.2f, 0.8f, 1.f), FRotator(90.f, 0.f, 0.f));
				break;
			default: break;
			}
			Fixtures.Add(V);
		}
	}

	Figures.SetNum(L.Actors.Num());
	for (int32 i = 0; i < L.Actors.Num(); ++i) BuildFigure(L.Actors[i], Figures[i]);

	SightMat = GlowMat(FLinearColor(1.f, 0.75f, 0.2f), 0.28f);
	SightAlertMat = GlowMat(FLinearColor(1.f, 0.08f, 0.04f), 0.6f);
	for (int32 i = 0; i < 24; ++i)
	{
		UStaticMeshComponent* P = AddMesh(nullptr, PlaneMesh, FVector::ZeroVector, FVector(0.9f * TS, 0.9f * TS, 1.f), SightMat, FRotator::ZeroRotator, false);
		P->SetVisibility(false);
		SightPool.Add(P);
	}

	ApplyLook(L.Reel);

	bCamInit = false;
	SetCamMode(CamMode, true);
}

void AMJStage::ComputeCamera(EMJCamMode Mode, FVector& OutLoc, FRotator& OutRot) const
{
	const float HalfW = BoardW * TileSize * 0.5f + 90.f;
	const float Depth = BoardH * TileSize + 160.f;
	const float Pitch = Mode == EMJCamMode::Screening ? CamPitch + 10.f : CamPitch;
	const float PitchRad = FMath::DegreesToRadians(-Pitch);
	const float TanH = FMath::Tan(FMath::DegreesToRadians(CamFov * 0.5f));
	const float TanV = TanH * 9.f / 16.f;
	const float Usable = Mode == EMJCamMode::Edit ? (1.f - PanelFraction - 0.10f) : 0.78f; // fraction of screen height for the board
	const float DistW = HalfW / TanH;
	const float DistH = (Depth * FMath::Sin(PitchRad)) / (2.f * TanV * Usable);
	const float Dist = FMath::Max(DistW, DistH) * (Mode == EMJCamMode::Title ? 1.6f : 1.0f);

	// push the board up the screen so the timeline panel does not cover it
	const float Up = Mode == EMJCamMode::Edit ? (PanelFraction * 0.5f - 0.04f) : -0.02f; // fraction of screen height
	const float ShiftX = Up * 2.f * TanV * Dist / FMath::Sin(PitchRad);

	OutRot = FRotator(Pitch, 0.f, 0.f);
	const FVector Target(-ShiftX, 0.f, 20.f);
	OutLoc = Target - OutRot.Vector() * Dist;
}

void AMJStage::SetCamMode(EMJCamMode Mode, bool bSnap)
{
	CamMode = Mode;
	CamTime = 0.f;
	if (bSnap || !bCamInit)
	{
		ComputeCamera(Mode, CamLoc, CamRot);
		Camera->SetRelativeLocationAndRotation(CamLoc, CamRot);
		bCamInit = true;
	}
}

void AMJStage::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector Loc; FRotator Rot;
	ComputeCamera(CamMode, Loc, Rot);
	CamTime += DeltaSeconds;
	if (CamMode == EMJCamMode::Screening)
	{
		// slow dolly so a screening never looks static
		Loc += FVector(0.f, FMath::Sin(CamTime * 0.35f) * 60.f, 0.f);
	}
	const float K = 1.f - FMath::Exp(-CamLerpSpeed * DeltaSeconds);
	CamLoc = FMath::Lerp(CamLoc, Loc, K);
	CamRot = FMath::Lerp(CamRot, Rot, K);
	Camera->SetRelativeLocationAndRotation(CamLoc, CamRot);
	Camera->SetFieldOfView(CamFov);

	// idle motion that does not depend on the simulation
	const float T = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	for (FMJFixtureVis& V : Fixtures)
	{
		if ((V.Fix == EMJFix::Item || V.Fix == EMJFix::Bonus) && V.Moving)
		{
			V.Moving->SetRelativeRotation(FRotator(0.f, T * 70.f, 0.f));
		}
	}
}

void AMJStage::Apply(const FMJLevelDef& L, const FMJSimResult& R, float Time)
{
	if (R.States.Num() == 0 || Figures.Num() != L.Actors.Num()) return;

	const int32 N = R.Steps.Num();
	const int32 B = N > 0 ? FMath::Clamp(FMath::FloorToInt(Time), 0, N - 1) : 0;
	const float A = N > 0 ? FMath::Clamp(Time - B, 0.f, 1.f) : 0.f;
	const FMJBeatState& S0 = R.States[B];
	const FMJBeatState& S1 = N > 0 ? R.States[B + 1] : S0;
	const bool bLastBeat = N > 0 && B == N - 1;

	// ---- figures
	for (int32 i = 0; i < Figures.Num(); ++i)
	{
		FMJFigure& F = Figures[i];
		if (!F.bOnSet || !F.Pivot) continue;
		const FMJActorState& A0 = S0.Actors[i];
		const FMJActorState& A1 = S1.Actors[i];
		const uint16 Flags = N > 0 ? R.Steps[B].Actors[i].Flags : 0;
		const EMJAct Act = N > 0 ? R.Steps[B].Actors[i].Act : EMJAct::Wait;

		const FVector P0 = TileToWorld(A0.Pos);
		const FVector P1 = TileToWorld(A1.Pos);
		FVector Pos = P0;
		float Hop = 0.f, Squash = 1.f, Lean = 0.f;
		const FVector Fwd = FRotator(0.f, DirYaw(A1.Facing), 0.f).Vector();

		if (Flags & MJStep_Moved)
		{
			const float U = FMath::Clamp(A / 0.72f, 0.f, 1.f);
			Pos = FMath::Lerp(P0, P1, Ease(U));
			Hop = FMath::Sin(PI * U) * HopHeight;
			Squash = 1.f + 0.12f * FMath::Sin(PI * U) - (U >= 1.f ? 0.10f * FMath::Sin(PI * FMath::Clamp((A - 0.72f) / 0.28f, 0.f, 1.f)) : 0.f);
			Lean = 10.f * FMath::Sin(PI * U);
		}
		else if (Flags & MJStep_Bumped)
		{
			const float U = FMath::Clamp(A / 0.5f, 0.f, 1.f);
			Pos = P0 + Fwd * 26.f * FMath::Sin(PI * U);
			Lean = -14.f * FMath::Sin(PI * U);
		}
		else if (Act == EMJAct::Attack)
		{
			const float U = FMath::Clamp(A / 0.45f, 0.f, 1.f);
			Pos = P0 + Fwd * 36.f * FMath::Sin(PI * U);
			Lean = 22.f * FMath::Sin(PI * U);
		}
		else if (Act == EMJAct::Use)
		{
			Squash = 1.f - 0.22f * FMath::Sin(PI * FMath::Clamp(A / 0.6f, 0.f, 1.f));
		}

		const float Yaw0 = DirYaw(A0.Facing);
		const float Yaw = Yaw0 + FMath::FindDeltaAngleDegrees(Yaw0, DirYaw(A1.Facing)) * Ease(A / 0.3f);

		// going down happens late in the beat, after the blow lands
		const bool bDown0 = A0.Status != EMJStatus::Active;
		const bool bDown1 = A1.Status != EMJStatus::Active;
		const float Down = bDown0 ? 1.f : (bDown1 ? Ease((A - 0.5f) / 0.4f) : 0.f);
		const EMJDeath Death = bDown1 ? A1.Death : EMJDeath::None;

		FVector Scale(1.f, 1.f, Squash);
		float Pitch = Lean, Roll = 0.f, Sink = 0.f; // Pitch = forward tilt in degrees
		if (Down > 0.f)
		{
			switch (Death)
			{
			case EMJDeath::Crush: Scale = FVector(1.f + 0.4f * Down, 1.f + 0.4f * Down, FMath::Lerp(1.f, 0.07f, Down)); break;
			case EMJDeath::Fall: Sink = -260.f * Down * Down; Scale = FVector(1.f - 0.5f * Down); break;
			case EMJDeath::Laser:
				Pitch = 86.f * Down; Roll = FMath::Sin(A * 60.f) * 6.f * (1.f - Down);
				break;
			default: Pitch = -86.f * Down; break; // knocked flat on its back
			}
		}

		F.Pivot->SetRelativeLocationAndRotation(Pos + FVector(0.f, 0.f, Hop + Sink), FRotator(0.f, Yaw, 0.f));
		F.Tilt->SetRelativeRotation(FRotator(-Pitch, 0.f, Roll));
		F.Tilt->SetRelativeScale3D(Scale);

		if (F.Alert) F.Alert->SetVisibility((Flags & MJStep_Spotter) && A > 0.55f);
		if (F.Carry) F.Carry->SetVisibility(Down < 0.5f && (A0.bHasItem || (A1.bHasItem && A > 0.55f)));
	}

	// ---- fixtures
	const FMJBeatState& Late = A > 0.6f ? S1 : S0;
	for (FMJFixtureVis& V : Fixtures)
	{
		const uint32 Bit = 1u << V.Chan;
		switch (V.Fix)
		{
		case EMJFix::Door:
		{
			const float O0 = (S0.Active & Bit) ? 1.f : 0.f, O1 = (S1.Active & Bit) ? 1.f : 0.f;
			const float Open = FMath::Lerp(O0, O1, Ease((A - 0.55f) / 0.4f));
			if (V.Moving) V.Moving->SetRelativeLocation(FVector(0.f, 0.f, -82.f * Open));
			break;
		}
		case EMJFix::Laser:
		{
			const bool bOn = !((A > 0.82f ? S1 : S0).Active & Bit);
			for (UStaticMeshComponent* Beam : V.Beams) { if (Beam) Beam->SetVisibility(bOn); }
			break;
		}
		case EMJFix::Plate:
		{
			bool bPressed = false;
			for (int32 i = 0; i < Late.Actors.Num(); ++i) { if (Figures[i].bOnSet && Late.Actors[i].Pos == V.Tile) bPressed = true; }
			if (V.Moving) V.Moving->SetRelativeLocation(FVector(0.f, 0.f, bPressed ? -3.f : 0.f));
			break;
		}
		case EMJFix::Lever:
		{
			const float T0 = (S0.Lever & Bit) ? 1.f : 0.f, T1 = (S1.Lever & Bit) ? 1.f : 0.f;
			const float T = FMath::Lerp(T0, T1, Ease((A - 0.25f) / 0.35f));
			if (V.Moving) V.Moving->SetRelativeRotation(FRotator(FMath::Lerp(-32.f, 32.f, T), 0.f, 0.f));
			break;
		}
		case EMJFix::Item:
			if (V.Moving) V.Moving->SetVisibility(!Late.bItemTaken, true);
			break;
		case EMJFix::Bonus:
			if (V.Moving) V.Moving->SetVisibility(!Late.bBonusTaken, true);
			break;
		default: break;
		}
	}

	// ---- sight lines
	int32 Used = 0;
	TArray<FIntPoint, TInlineAllocator<8>> Tiles;
	const FMJBeatState& See = A > 0.35f ? S1 : S0;
	const bool bSpottedNow = R.Outcome == EMJOutcome::Spotted && bLastBeat && A > 0.55f;
	for (int32 i = 0; i < L.Actors.Num(); ++i)
	{
		MJ::SightTiles(L, See, i, Tiles);
		for (const FIntPoint& P : Tiles)
		{
			if (Used >= SightPool.Num()) break;
			UStaticMeshComponent* Plane = SightPool[Used++];
			Plane->SetRelativeLocation(TileToWorld(P, 2.5f));
			Plane->SetMaterial(0, (bSpottedNow && i == R.Culprit) ? SightAlertMat : SightMat);
			Plane->SetVisibility(true);
		}
	}
	for (int32 i = Used; i < SightPool.Num(); ++i) SightPool[i]->SetVisibility(false);
}
