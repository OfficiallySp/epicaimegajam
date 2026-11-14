// Fix It In Post - the miniature set.
#include "Game/MJStage.h"

#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
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

	/** The glow material is tuned for thin beams; solid shapes need far less boost or they bloom into a blob. */
	UMaterialInstanceDynamic* Dimmed(UMaterialInstanceDynamic* M, float Boost)
	{
		M->SetScalarParameterValue(TEXT("Boost"), Boost);
		return M;
	}

	FLinearColor ChanColor(int32 Chan)
	{
		static const FLinearColor Colors[4] = {
			FLinearColor(1.0f, 0.50f, 0.06f), FLinearColor(0.06f, 0.72f, 0.9f), FLinearColor(0.9f, 0.12f, 0.6f), FLinearColor(0.4f, 0.85f, 0.12f) };
		return Colors[FMath::Clamp(Chan, 0, 3)];
	}

	struct FReelLook
	{
		FLinearColor FloorA, FloorB, FloorTrim, Wall, WallTrim, Backdrop, Accent;
	};

	const FReelLook& ReelLook(int32 Reel)
	{
		static const FReelLook Looks[3] = {
			// the museum: warm stone, navy walls, teal backdrop
			{ FLinearColor(0.56f, 0.47f, 0.35f), FLinearColor(0.47f, 0.39f, 0.29f), FLinearColor(0.20f, 0.16f, 0.12f), FLinearColor(0.06f, 0.09f, 0.17f), FLinearColor(0.03f, 0.04f, 0.08f), FLinearColor(0.05f, 0.24f, 0.30f), FLinearColor(0.10f, 0.55f, 1.0f) },
			// the getaway: concrete, brick, dusk purple
			{ FLinearColor(0.38f, 0.37f, 0.37f), FLinearColor(0.30f, 0.29f, 0.30f), FLinearColor(0.12f, 0.12f, 0.13f), FLinearColor(0.17f, 0.07f, 0.06f), FLinearColor(0.07f, 0.03f, 0.03f), FLinearColor(0.20f, 0.09f, 0.26f), FLinearColor(1.0f, 0.15f, 0.65f) },
			// the lair: dark steel, blood red
			{ FLinearColor(0.21f, 0.23f, 0.28f), FLinearColor(0.16f, 0.17f, 0.21f), FLinearColor(0.05f, 0.05f, 0.07f), FLinearColor(0.10f, 0.03f, 0.04f), FLinearColor(0.04f, 0.01f, 0.02f), FLinearColor(0.32f, 0.04f, 0.05f), FLinearColor(1.0f, 0.16f, 0.08f) },
		};
		return Looks[FMath::Clamp(Reel - 1, 0, 2)];
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
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMatF(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	static ConstructorHelpers::FObjectFinder<UTextureCube> SkyF(TEXT("/Engine/EngineResources/GrayDarkTextureCube.GrayDarkTextureCube"));
	CubeMesh = CubeF.Object;
	CylMesh = CylF.Object;
	ConeMesh = ConeF.Object;
	SphereMesh = SphereF.Object;
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

	LampLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("LampLight"));
	LampLight->SetupAttachment(Root);
	LampLight->SetMobility(EComponentMobility::Movable);
	LampLight->IntensityUnits = ELightUnits::Candelas;

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(Root);
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->SourceType = ESkyLightSourceType::SLS_SpecifiedCubemap;
	SkyLight->Cubemap = SkyCube;
}

void AMJStage::ApplyLook(int32 Reel)
{
	const FReelLook& Look = ReelLook(Reel);

	FPostProcessSettings& PP = Camera->PostProcessSettings;
	PP.bOverride_AutoExposureMethod = true; PP.AutoExposureMethod = AEM_Manual;
	PP.bOverride_AutoExposureBias = true; PP.AutoExposureBias = -ExposureEV; // manual exposure sits at EV100 0, so the bias is the whole exposure
	PP.bOverride_AutoExposureApplyPhysicalCameraExposure = true; PP.AutoExposureApplyPhysicalCameraExposure = false;
	PP.bOverride_VignetteIntensity = true; PP.VignetteIntensity = 0.55f;
	PP.bOverride_MotionBlurAmount = true; PP.MotionBlurAmount = 0.f;
	PP.bOverride_BloomIntensity = true; PP.BloomIntensity = 0.9f;
	PP.bOverride_FilmGrainIntensity = true; PP.FilmGrainIntensity = 0.28f;
	PP.bOverride_ColorSaturation = true; PP.ColorSaturation = FVector4(1.08f, 1.08f, 1.08f, 1.f);
	PP.bOverride_ColorContrast = true; PP.ColorContrast = FVector4(1.05f, 1.05f, 1.05f, 1.f);
	PP.bOverride_DepthOfFieldSensorWidth = true; PP.DepthOfFieldSensorWidth = DofSensorWidth;
	PP.bOverride_DepthOfFieldFstop = true; PP.DepthOfFieldFstop = DofFstop;
	PP.bOverride_DepthOfFieldFocalDistance = true; PP.DepthOfFieldFocalDistance = CamFocus;

	KeyLight->SetRelativeLocation(FVector(-650.f, -1050.f, 1750.f));
	KeyLight->SetIntensityUnits(ELightUnits::Candelas);
	KeyLight->SetIntensity(KeyCandelas);
	KeyLight->SetAttenuationRadius(9000.f);
	KeyLight->SetInnerConeAngle(22.f);
	KeyLight->SetOuterConeAngle(42.f);
	KeyLight->SetSourceRadius(45.f);
	KeyLight->SetLightColor(FLinearColor(1.0f, 0.80f, 0.58f));
	KeyLight->SetCastShadows(true);
	KeyLight->SetRelativeRotation((FVector(0.f, 0.f, 0.f) - KeyLight->GetRelativeLocation()).Rotation());

	FillLight->SetRelativeLocation(FVector(-1100.f, 1500.f, 800.f));
	FillLight->SetIntensityUnits(ELightUnits::Candelas);
	FillLight->SetIntensity(FillCandelas);
	FillLight->SetAttenuationRadius(9000.f);
	FillLight->SetSourceRadius(150.f);
	FillLight->SetLightColor(Look.Accent);
	FillLight->SetCastShadows(false);

	LampLight->SetIntensityUnits(ELightUnits::Candelas);
	LampLight->SetIntensity(LampCandelas);
	LampLight->SetAttenuationRadius(2600.f);
	LampLight->SetSourceRadius(40.f);
	LampLight->SetLightColor(FLinearColor(1.0f, 0.72f, 0.42f));
	LampLight->SetCastShadows(false);

	SkyLight->SetIntensity(SkyIntensity);
	SkyLight->SetLightColor(FLinearColor(0.45f, 0.55f, 1.0f));
}

FVector AMJStage::TileToWorld(FIntPoint P, float Z) const
{
	return FVector(-(P.Y - (BoardH - 1) * 0.5f) * TileSize, (P.X - (BoardW - 1) * 0.5f) * TileSize, Z);
}

// ---------------------------------------------------------------------------------------------------------------------
// building blocks

UStaticMesh* AMJStage::LoadMesh(const FString& ObjectPath)
{
	const FName Key(*ObjectPath);
	if (TObjectPtr<UStaticMesh>* Found = MeshCache.Find(Key)) return Found->Get();
	UStaticMesh* Mesh = LoadQuiet<UStaticMesh>(*ObjectPath);
	MeshCache.Add(Key, Mesh);
	return Mesh;
}

UStaticMesh* AMJStage::Kit(const TCHAR* Name)
{
	UStaticMesh* Mesh = LoadMesh(FString::Printf(TEXT("/Game/MegaJam26/Kit/SM_%s.SM_%s"), Name, Name));
	return Mesh ? Mesh : CubeMesh.Get();
}

void AMJStage::ApplySlots(UMeshComponent* Comp, UStaticMesh* Mesh, std::initializer_list<FSlotMat> Slots) const
{
	if (!Comp || !Mesh) return;
	const TArray<FStaticMaterial>& MeshSlots = Mesh->GetStaticMaterials();
	for (int32 i = 0; i < MeshSlots.Num(); ++i)
	{
		for (const FSlotMat& S : Slots)
		{
			if (S.Mat && MeshSlots[i].MaterialSlotName == FName(S.Slot)) Comp->SetMaterial(i, S.Mat);
		}
	}
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

UStaticMeshComponent* AMJStage::AddKit(USceneComponent* Parent, const TCHAR* Name, const FVector& Loc, std::initializer_list<FSlotMat> Slots, const FRotator& Rot, const FVector& Scale, bool bShadow)
{
	UStaticMesh* Mesh = Kit(Name);
	UStaticMeshComponent* C = AddMesh(Parent, Mesh, Loc, Scale, nullptr, Rot, bShadow);
	ApplySlots(C, Mesh, Slots);
	return C;
}

UInstancedStaticMeshComponent* AMJStage::AddInstanced(const TCHAR* Name, std::initializer_list<FSlotMat> Slots)
{
	UStaticMesh* Mesh = Kit(Name);
	UInstancedStaticMeshComponent* C = NewObject<UInstancedStaticMeshComponent>(this);
	C->SetMobility(EComponentMobility::Movable);
	C->SetStaticMesh(Mesh);
	C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	C->SetupAttachment(SetRoot);
	C->RegisterComponent();
	ApplySlots(C, Mesh, Slots);
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

void AMJStage::AddProp(const TCHAR* Name, const FVector& Loc, float Yaw, float LongestSide)
{
	UStaticMesh* Mesh = LoadMesh(FString::Printf(TEXT("/Game/MegaJam26/Props/SM_%s.SM_%s"), Name, Name));
	if (!Mesh) return;
	const FBoxSphereBounds B = Mesh->GetBounds();
	const float Scale = LongestSide / FMath::Max(1.f, 2.f * B.BoxExtent.GetMax());
	const FRotator Rot(0.f, Yaw, 0.f);
	const FVector Foot = B.Origin - FVector(0.f, 0.f, B.BoxExtent.Z);
	AddMesh(nullptr, Mesh, Loc - Rot.RotateVector(Foot * Scale), FVector(Scale), nullptr, Rot);
}

UMaterialInstanceDynamic* AMJStage::Solid(const FLinearColor& Color, float Rough, float Glow, float Metal)
{
	UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(SolidBase, this);
	M->SetVectorParameterValue(TEXT("Color"), Color);
	M->SetScalarParameterValue(TEXT("Rough"), Rough);
	M->SetScalarParameterValue(TEXT("Glow"), Glow);
	M->SetScalarParameterValue(TEXT("Metal"), Metal);
	Mats.Add(M);
	return M;
}

UMaterialInstanceDynamic* AMJStage::GlowMat(const FLinearColor& Color, float Opacity)
{
	UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(GlowBase, this);
	M->SetVectorParameterValue(TEXT("Color"), Color);
	M->SetScalarParameterValue(TEXT("Opacity"), Opacity);
	Mats.Add(M);
	return M;
}

void AMJStage::ClearDynamic()
{
	for (USceneComponent* C : Dynamic) { if (C) C->DestroyComponent(); }
	Dynamic.Reset();
	Mats.Reset();
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

	float Height = FigureHeight;
	const FString MeshPath = FString::Printf(TEXT("/Game/MegaJam26/Characters/SM_%s.SM_%s"), *D.Look.ToString(), *D.Look.ToString());
	if (UStaticMesh* Custom = LoadMesh(MeshPath))
	{
		// Generated figurines arrive at any size and pivot: stand them on the tile at a common size.
		const FBoxSphereBounds B = Custom->GetBounds();
		const bool bCar = D.Role == EMJRole::Crusher;
		Height = bCar ? 62.f : FigureHeight;
		const float Scale = bCar ? 118.f / FMath::Max(1.f, 2.f * FMath::Max(B.BoxExtent.X, B.BoxExtent.Y)) : Height / FMath::Max(1.f, B.BoxExtent.Z * 2.f);
		const FRotator Rot(0.f, bCar ? CarYaw : FigureYaw, 0.f);
		const FVector Foot = B.Origin - FVector(0.f, 0.f, B.BoxExtent.Z);
		AddMesh(Out.Tilt, Custom, -Rot.RotateVector(Foot * Scale), FVector(Scale), nullptr, Rot);
	}
	else
	{
		// stand-in pawn: base, body, head and a visor that shows the facing
		FLinearColor Body(0.8f, 0.8f, 0.8f), Base(0.08f, 0.08f, 0.09f), Accent(1.f, 1.f, 1.f);
		switch (D.Role)
		{
		case EMJRole::Star: Body = FLinearColor(0.92f, 0.9f, 0.84f); Accent = FLinearColor(1.0f, 0.72f, 0.1f); Base = FLinearColor(0.75f, 0.52f, 0.08f); break;
		case EMJRole::Ally: Body = FLinearColor(0.1f, 0.55f, 0.6f); Accent = FLinearColor(0.95f, 0.85f, 0.7f); Base = FLinearColor(0.05f, 0.3f, 0.34f); break;
		case EMJRole::Guard: Body = FLinearColor(0.12f, 0.14f, 0.32f); Accent = FLinearColor(0.85f, 0.2f, 0.15f); Base = FLinearColor(0.35f, 0.06f, 0.05f); break;
		case EMJRole::Villain: Body = FLinearColor(0.55f, 0.04f, 0.08f); Accent = FLinearColor(0.9f, 0.75f, 0.65f); Base = FLinearColor(0.2f, 0.02f, 0.03f); break;
		default: break;
		}
		AddMesh(Out.Tilt, CylMesh, FVector(0.f, 0.f, 4.f), FVector(0.66f, 0.66f, 0.08f), Solid(Base, 0.35f));
		AddMesh(Out.Tilt, ConeMesh, FVector(0.f, 0.f, 44.f), FVector(0.5f, 0.5f, 0.72f), Solid(Body, 0.55f));
		AddMesh(Out.Tilt, SphereMesh, FVector(0.f, 0.f, 88.f), FVector(0.3f), Solid(Accent, 0.5f));
		AddMesh(Out.Tilt, CubeMesh, FVector(15.f, 0.f, 90.f), FVector(0.1f, 0.2f, 0.06f), Solid(FLinearColor(0.02f, 0.02f, 0.02f), 0.3f));
	}

	Out.Alert = AddKit(Out.Pivot, TEXT("alert_mark"), FVector(0.f, 0.f, Height + 14.f), { { TEXT("Glow"), Dimmed(GlowMat(FLinearColor(1.f, 0.10f, 0.04f), 1.f), 1.4f) } }, FRotator(0.f, 90.f, 0.f), FVector(1.5f), false);
	Out.Alert->SetVisibility(false);
	Out.Carry = AddKit(Out.Pivot, TEXT("gem"), FVector(0.f, 0.f, Height + 26.f), { { TEXT("Glow"), Solid(FLinearColor(0.30f, 0.85f, 1.f), 0.08f, 0.9f) } }, FRotator::ZeroRotator, FVector(0.5f), false);
	Out.Carry->SetVisibility(false);

	Out.Spinner = AddPivot(Out.Pivot, FVector(0.f, 0.f, 62.f));
	UMaterialInstanceDynamic* StarMat = Solid(FLinearColor(1.f, 0.8f, 0.15f), 0.3f, 3.f);
	for (int32 i = 0; i < 3; ++i)
	{
		const float Ang = i * 2.f * PI / 3.f;
		AddKit(Out.Spinner, TEXT("star"), FVector(FMath::Cos(Ang) * 34.f, FMath::Sin(Ang) * 34.f, 0.f), { { TEXT("Glow"), StarMat } }, FRotator(0.f, 90.f, 0.f), FVector(1.f), false);
	}
	Out.Spinner->SetVisibility(false, true);
}

// ---------------------------------------------------------------------------------------------------------------------
// the set

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

	const FReelLook& Look = ReelLook(L.Reel);
	UMaterialInstanceDynamic* Wood = Solid(FLinearColor(0.11f, 0.065f, 0.04f), 0.5f);
	UMaterialInstanceDynamic* DeskWood = Solid(FLinearColor(0.19f, 0.105f, 0.055f), 0.42f);
	UMaterialInstanceDynamic* Metal = Solid(FLinearColor(0.55f, 0.55f, 0.58f), 0.28f, 0.f, 1.f);
	UMaterialInstanceDynamic* Dark = Solid(FLinearColor(0.035f, 0.035f, 0.045f), 0.45f);
	UMaterialInstanceDynamic* FloorA = Solid(Look.FloorA, 0.72f);
	UMaterialInstanceDynamic* FloorB = Solid(Look.FloorB, 0.72f);
	UMaterialInstanceDynamic* FloorTrim = Solid(Look.FloorTrim, 0.6f);
	UMaterialInstanceDynamic* WallPaint = Solid(Look.Wall, 0.8f);
	UMaterialInstanceDynamic* WallTrim = Solid(Look.WallTrim, 0.7f);
	// the flats carry a painted atlas (four panels per reel, tools/make_backdrops.py); plain paint if it is missing
	UMaterialInterface* Backdrop = Solid(Look.Backdrop, 0.85f);
	if (UMaterialInterface* BackdropBase = LoadQuiet<UMaterialInterface>(TEXT("/Game/MegaJam26/Materials/M_Backdrop.M_Backdrop")))
	{
		const int32 R = FMath::Clamp(L.Reel, 1, 3);
		if (UTexture* Atlas = LoadQuiet<UTexture>(*FString::Printf(TEXT("/Game/MegaJam26/Environment/T_Backdrop_R%d.T_Backdrop_R%d"), R, R)))
		{
			UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(BackdropBase, this);
			M->SetTextureParameterValue(TEXT("Atlas"), Atlas);
			Mats.Add(M);
			Backdrop = M;
		}
	}
	UMaterialInstanceDynamic* FlatWood = Solid(FLinearColor(0.42f, 0.28f, 0.15f), 0.7f);

	// the desk and the plinth the set stands on
	const float DeskZ = -40.f;
	AddKit(nullptr, TEXT("desk"), FVector(0.f, 0.f, DeskZ), { { TEXT("Wood"), DeskWood } }, FRotator::ZeroRotator, FVector(90.f, 90.f, 1.f));
	AddKit(nullptr, TEXT("plinth"), FVector::ZeroVector, { { TEXT("Wood"), Wood } }, FRotator::ZeroRotator, FVector(BoardH + 0.5f, BoardW + 0.5f, 1.f));

	UInstancedStaticMeshComponent* TilesA = AddInstanced(TEXT("floor_tile"), { { TEXT("Paint"), FloorA }, { TEXT("Trim"), FloorTrim } });
	UInstancedStaticMeshComponent* TilesB = AddInstanced(TEXT("floor_tile_b"), { { TEXT("Paint"), FloorB }, { TEXT("Trim"), FloorTrim } });
	UInstancedStaticMeshComponent* Walls = AddInstanced(TEXT("wall_block"), { { TEXT("Paint"), WallPaint }, { TEXT("Trim"), WallTrim } });
	UInstancedStaticMeshComponent* Flats = AddInstanced(TEXT("flat_backdrop"), { { TEXT("Backdrop"), Backdrop }, { TEXT("Wood"), FlatWood } });
	Flats->SetNumCustomDataFloats(1); // which of the four painted panels this flat shows

	auto IsOpen = [&L](FIntPoint P) { return L.InBounds(P) && L.Cell(P).Tile != EMJTile::Wall && L.Cell(P).Tile != EMJTile::Void; };

	for (int32 Y = 0; Y < L.H; ++Y)
	{
		for (int32 X = 0; X < L.W; ++X)
		{
			const FIntPoint P(X, Y);
			const FMJCell& C = L.Cell(P);
			if (C.Tile == EMJTile::Void) continue;
			if (C.Tile == EMJTile::Wall)
			{
				// only the walls that border the playable floor are built; the rest of the block is just plinth
				bool bBorders = false;
				for (int32 DY = -1; DY <= 1; ++DY) for (int32 DX = -1; DX <= 1; ++DX) bBorders |= IsOpen(P + FIntPoint(DX, DY));
				if (bBorders) Walls->AddInstance(FTransform(TileToWorld(P)));
				continue;
			}
			if (C.Tile == EMJTile::Pit)
			{
				AddMesh(nullptr, CubeMesh, TileToWorld(P, -9.f), FVector(0.98f, 0.98f, 0.02f), Solid(FLinearColor(0.004f, 0.004f, 0.006f), 0.95f), FRotator::ZeroRotator, false);
			}
			else
			{
				(((X + Y) & 1) ? TilesA : TilesB)->AddInstance(FTransform(TileToWorld(P)));
			}
			// a painted flat stands along the far edge wherever the floor meets a wall
			if (!IsOpen(P + FIntPoint(0, -1)))
			{
				const int32 Flat = Flats->AddInstance(FTransform(FRotator(0.f, 90.f, 0.f), TileToWorld(P), FVector(1.f, 1.f, FlatHeightScale)));
				Flats->SetCustomDataValue(Flat, 0, (float)((X + 2 * Y) % 4), true);
			}

			if (C.Fix == EMJFix::None) continue;
			FMJFixtureVis V;
			V.Tile = P; V.Fix = C.Fix; V.Chan = C.Chan;
			V.Root = AddPivot(nullptr, TileToWorld(P));
			const FLinearColor CC = ChanColor(C.Chan);
			switch (C.Fix)
			{
			case EMJFix::Door:
				AddKit(V.Root, TEXT("gate_frame"), FVector::ZeroVector, { { TEXT("Metal"), Metal } });
				V.Moving = AddPivot(V.Root, FVector::ZeroVector);
				AddKit(V.Moving, TEXT("gate_block"), FVector::ZeroVector, { { TEXT("Paint"), Solid(CC * 0.35f, 0.5f) }, { TEXT("Accent"), Solid(CC, 0.3f, 1.6f) }, { TEXT("Trim"), Dark } });
				break;
			case EMJFix::Plate:
				AddKit(V.Root, TEXT("plate_base"), FVector::ZeroVector, { { TEXT("Trim"), Dark } });
				V.Moving = AddPivot(V.Root, FVector(0.f, 0.f, 1.f));
				AddKit(V.Moving, TEXT("plate_top"), FVector::ZeroVector, { { TEXT("Accent"), Solid(CC, 0.35f, 0.9f) } });
				break;
			case EMJFix::Lever:
				AddKit(V.Root, TEXT("lever_base"), FVector::ZeroVector, { { TEXT("Trim"), Dark }, { TEXT("Metal"), Metal } });
				V.Moving = AddPivot(V.Root, FVector(-28.f, 0.f, 12.f));
				AddKit(V.Moving, TEXT("lever_arm"), FVector::ZeroVector, { { TEXT("Metal"), Metal }, { TEXT("Accent"), Solid(CC, 0.3f, 1.8f) } });
				break;
			case EMJFix::Laser:
			{
				AddKit(V.Root, TEXT("laser_frame"), FVector::ZeroVector, { { TEXT("Trim"), Dark }, { TEXT("Metal"), Metal }, { TEXT("Glow"), Solid(CC, 0.3f, 4.f) } });
				static const FVector2D Nozzles[4] = { FVector2D(-33.f, 20.f), FVector2D(-11.f, -7.f), FVector2D(11.f, 7.f), FVector2D(33.f, -20.f) };
				UMaterialInstanceDynamic* Beam = GlowMat(CC, 0.95f); // lasers wear their channel colour so the lever or grid that controls them is obvious
				for (const FVector2D& N : Nozzles)
				{
					V.Beams.Add(AddMesh(V.Root, CylMesh, FVector(N.X, N.Y, 55.f), FVector(0.07f, 0.07f, 1.0f), Beam, FRotator::ZeroRotator, false));
				}
				break;
			}
			case EMJFix::Item:
				AddKit(V.Root, TEXT("pedestal"), FVector(30.f, 0.f, 0.f), { { TEXT("Paint"), Solid(FLinearColor(0.75f, 0.72f, 0.66f), 0.5f) }, { TEXT("Trim"), Dark }, { TEXT("Accent"), Solid(FLinearColor(0.45f, 0.03f, 0.06f), 0.9f) } });
				V.Moving = AddPivot(V.Root, FVector(30.f, 0.f, 52.f));
				AddKit(V.Moving, TEXT("gem"), FVector::ZeroVector, { { TEXT("Glow"), Solid(FLinearColor(0.30f, 0.85f, 1.f), 0.06f, 0.8f) } }, FRotator::ZeroRotator, FVector(1.15f));
				break;
			case EMJFix::Exit:
				AddKit(V.Root, TEXT("mark_tape"), FVector::ZeroVector, { { TEXT("Accent"), Solid(FLinearColor(1.f, 0.78f, 0.05f), 0.55f, 1.6f) } }, FRotator::ZeroRotator, FVector(1.25f), false);
				break;
			case EMJFix::Bonus:
				V.Moving = AddPivot(V.Root, FVector(0.f, 0.f, 42.f));
				AddKit(V.Moving, TEXT("reel_gold"), FVector::ZeroVector, { { TEXT("Metal"), Solid(FLinearColor(1.f, 0.72f, 0.1f), 0.22f, 0.7f, 1.f) } });
				break;
			default: break;
			}
			Fixtures.Add(V);
		}
	}

	StarFigure = L.StarIndex();
	Coverage.Reset();
	Shot = -1;
	Figures.SetNum(L.Actors.Num());
	for (int32 i = 0; i < L.Actors.Num(); ++i) BuildFigure(L.Actors[i], Figures[i]);

	SightEdge = GlowMat(FLinearColor(1.f, 0.72f, 0.15f), 0.85f);
	SightFill = GlowMat(FLinearColor(1.f, 0.72f, 0.15f), 0.16f);
	AlertEdge = GlowMat(FLinearColor(1.f, 0.08f, 0.04f), 1.f);
	AlertFill = GlowMat(FLinearColor(1.f, 0.08f, 0.04f), 0.5f);
	for (int32 i = 0; i < 24; ++i)
	{
		UStaticMeshComponent* P = AddKit(nullptr, TEXT("sight_tile"), FVector::ZeroVector, { { TEXT("Glow"), SightEdge }, { TEXT("Accent"), SightFill } }, FRotator::ZeroRotator, FVector(1.f), false);
		P->SetVisibility(false);
		SightPool.Add(P);
	}

	// ---- desk dressing: real objects around the toy set (they are what sells the scale)
	const float HX = BoardH * TileSize * 0.5f, HY = BoardW * TileSize * 0.5f;
	const FVector LampAt(HX + 330.f, -HY - 420.f, DeskZ);
	AddProp(TEXT("Lamp"), LampAt, LampYaw, 1050.f);
	LampLight->SetRelativeLocation(LampAt + FVector(-120.f, 170.f, 700.f));
	AddProp(TEXT("Clapper"), FVector(HX + 360.f, HY * 0.35f, DeskZ), 80.f, 560.f);
	AddProp(TEXT("Filmcan"), FVector(HX + 250.f, HY + 330.f, DeskZ), 25.f, 400.f);
	AddProp(TEXT("Reel"), FVector(-HX + 40.f, -HY - 370.f, DeskZ), 10.f, 380.f);
	AddProp(TEXT("Mug"), FVector(-HX + 120.f, HY + 300.f, DeskZ), 200.f, 210.f);
	AddProp(TEXT("Scissors"), FVector(-HX - 260.f, HY * 0.5f, DeskZ), 150.f, 420.f);

	ApplyLook(L.Reel);

	bCamInit = false;
	SetCamMode(CamMode, true);
}

// ---------------------------------------------------------------------------------------------------------------------
// camera

void AMJStage::ComputeCamera(EMJCamMode Mode, FVector& OutLoc, FRotator& OutRot, float& OutFocus) const
{
	const float HalfW = BoardW * TileSize * 0.5f + 90.f;
	const float Depth = BoardH * TileSize + 200.f;
	const float Pitch = Mode == EMJCamMode::Screening ? CamPitch + 8.f : (Mode == EMJCamMode::Title ? CamPitch + 14.f : CamPitch);
	const float PitchRad = FMath::DegreesToRadians(-Pitch);
	const float TanH = FMath::Tan(FMath::DegreesToRadians(CamFov * 0.5f));
	const float TanV = TanH * 9.f / 16.f;
	const float Usable = Mode == EMJCamMode::Title ? 0.7f : (1.f - PanelFraction - 0.12f); // fraction of screen height for the board
	const float DistW = HalfW / TanH;
	const float DistH = (Depth * FMath::Sin(PitchRad)) / (2.f * TanV * Usable);
	const float Dist = FMath::Max(DistW, DistH) * (Mode == EMJCamMode::Title ? 1.2f : (Mode == EMJCamMode::Screening ? 0.94f : 1.0f));

	// push the board up the screen so the timeline panel does not cover it (the panel stays up during screenings)
	const float Up = Mode == EMJCamMode::Title ? -0.04f : (PanelFraction * 0.5f - 0.05f); // fraction of screen height
	const float ShiftX = Up * 2.f * TanV * Dist / FMath::Sin(PitchRad);
	const float ShiftY = Mode == EMJCamMode::Title ? -0.19f * 2.f * TanH * Dist : 0.f; // title: set on the right, text on the left

	OutRot = FRotator(Pitch, 0.f, 0.f);
	const FVector Target(-ShiftX, ShiftY, 20.f);
	OutLoc = Target - OutRot.Vector() * Dist;
	OutFocus = Dist + ShiftX * FMath::Cos(PitchRad); // keep the middle of the set, not the aim point, in focus

	// coverage: when the cut works the screening is shot like a film, from closer angles on whatever the action is
	FVector Subject;
	if (Mode == EMJCamMode::Screening && Coverage.IsValidIndex(Shot) && Coverage[Shot].Kind != 0 && CoverageCenter(Coverage[Shot], Subject))
	{
		const FMJCoverShot& S = Coverage[Shot];
		const bool bLow = S.Kind == 1;
		const FVector Offset = (bLow ? FVector(-640.f, -430.f, 470.f) : FVector(-600.f, 540.f, 620.f)) * S.Zoom;
		const FVector LookAt = Subject + FVector(50.f, bLow ? 40.f : -30.f, 55.f);
		OutLoc = Subject + Offset;
		OutRot = (LookAt - OutLoc).Rotation();
		// tilt down a touch so the subject sits in the part of the frame the timeline panel leaves free
		OutRot.Pitch -= PanelFraction * 0.5f * FMath::RadiansToDegrees(2.f * FMath::Atan(TanV));
		OutFocus = FVector::Dist(OutLoc, Subject + FVector(0.f, 0.f, 55.f));
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// coverage

namespace
{
	// what the audience must see a figure do: strong actions get a shot of their own, weak ones only join the star's
	constexpr uint16 MJStrongFlags = MJStep_Pulled | MJStep_Grabbed | MJStep_Hit | MJStep_Downed | MJStep_Bonus | MJStep_Arrived;
	constexpr uint16 MJWeakFlags = MJStep_Missed | MJStep_Bumped | MJStep_Turned;
	constexpr float MJMaxZoom = 1.75f; // past this a close shot is nearly as wide as the master, so use the master

	struct FMJWant
	{
		int32 Mode = 1;          // 0 = the master, 1 = on the star, 2 = a cutaway to action away from the star
		int32 Pri = 0;           // 3 = action away from the star, 2 = action next to it, 1 = lead-in or aftermath, 0 = quiet
		TArray<int32> Figs;      // figures to keep in frame (the star is one of them in mode 1)
		TArray<FIntPoint> Tiles; // set pieces to keep in frame
	};

	void MJUnion(FMJWant& A, const FMJWant& B)
	{
		for (int32 F : B.Figs) A.Figs.AddUnique(F);
		for (const FIntPoint& T : B.Tiles) A.Tiles.AddUnique(T);
		A.Pri = FMath::Max(A.Pri, B.Pri);
	}
}

bool AMJStage::CoverageCenter(const FMJCoverShot& S, FVector& Out) const
{
	FVector Sum = FVector::ZeroVector;
	int32 Count = 0;
	for (int32 F : S.Follow)
	{
		if (!Figures.IsValidIndex(F) || !Figures[F].Pivot) continue;
		const FVector P = Figures[F].Pivot->GetRelativeLocation();
		Sum += FVector(P.X, P.Y, 0.f); // not the hop: the camera should not bob with the figurines
		++Count;
	}
	for (const FVector& P : S.Fixed) { Sum += P; ++Count; }
	if (Count == 0) return false;
	Out = Sum / (float)Count;
	return true;
}

void AMJStage::PlanCoverage(const FMJLevelDef& L, const FMJSimResult& R)
{
	Coverage.Reset();
	Shot = -1;
	const int32 N = FMath::Min(R.EndBeat, R.Steps.Num());
	const int32 Star = StarFigure;
	if (N <= 0 || R.States.Num() < N + 1 || Figures.Num() != L.Actors.Num() || !Figures.IsValidIndex(Star) || !Figures[Star].bOnSet) return;

	auto Pos = [&R](int32 Fig, int32 State) { return R.States[State].Actors[Fig].Pos; };
	auto Near = [](FIntPoint A, FIntPoint B) { return FMath::Max(FMath::Abs(A.X - B.X), FMath::Abs(A.Y - B.Y)) <= 1; };

	// how far a shot must pull back to keep everything in it in frame, at every beat boundary it covers
	auto ZoomFor = [&](const FMJWant& W, int32 First, int32 Last)
	{
		float Reach = 0.f;
		for (int32 s = First; s <= Last + 1; ++s)
		{
			TArray<FVector2f, TInlineAllocator<16>> P;
			for (int32 F : W.Figs) { const FIntPoint Q = Pos(F, s); P.Add(FVector2f((float)Q.X, (float)Q.Y)); }
			for (const FIntPoint& T : W.Tiles) P.Add(FVector2f((float)T.X, (float)T.Y));
			if (P.Num() < 2) continue;
			FVector2f C(0.f, 0.f);
			for (const FVector2f& Q : P) C += Q;
			C /= (float)P.Num();
			for (const FVector2f& Q : P)
			{
				// grid X runs across the frame, grid Y into it, and the timeline panel leaves less room in depth
				const FVector2f D = (Q - C) * TileSize;
				Reach = FMath::Max(Reach, FMath::Sqrt(D.X * D.X + 2.25f * D.Y * D.Y));
			}
		}
		return FMath::Max(1.f, (Reach + 40.f) / 130.f);
	};

	// 1. where the action is, beat by beat
	TArray<FMJWant> Want;
	Want.SetNum(N);
	for (int32 b = 0; b < N; ++b)
	{
		FMJWant& W = Want[b];
		W.Figs.Add(Star);
		bool bAway = false, bStarActs = false;
		auto NearStar = [&](FIntPoint P) { return Near(P, Pos(Star, b)) || Near(P, Pos(Star, b + 1)); };
		auto AddFig = [&](int32 A, bool bStrong)
		{
			if (!Figures.IsValidIndex(A) || !Figures[A].bOnSet || A == Star) return;
			const bool bClose = NearStar(Pos(A, b)) || NearStar(Pos(A, b + 1));
			if (!bStrong && !bClose) return; // a guard turning on the far side of the set is not worth a cut
			W.Figs.AddUnique(A);
			W.Pri = FMath::Max(W.Pri, 2);
			bAway |= !bClose;
		};
		for (int32 a = 0; a < L.Actors.Num(); ++a)
		{
			const FMJActorStep& St = R.Steps[b].Actors[a];
			const uint16 Strong = MJStrongFlags | (L.Actors[a].Role == EMJRole::Crusher ? (uint16)MJStep_Moved : (uint16)0); // a car is a set piece whenever it drives
			if (!(St.Flags & (Strong | MJWeakFlags))) continue;
			if (a == Star) { W.Pri = FMath::Max(W.Pri, 2); bStarActs |= (St.Flags & Strong) != 0; }
			else AddFig(a, (St.Flags & Strong) != 0);
			if ((St.Flags & (MJStep_Hit | MJStep_Bumped)) && St.Other >= 0) AddFig(St.Other, (St.Flags & MJStep_Hit) != 0);
		}
		// doors, lasers, levers and plates that change state this beat, whoever or whatever flipped them
		const uint32 Changed = R.States[b].Active ^ R.States[b + 1].Active;
		for (const FMJFixtureVis& V : Fixtures)
		{
			if (V.Fix != EMJFix::Door && V.Fix != EMJFix::Laser && V.Fix != EMJFix::Lever && V.Fix != EMJFix::Plate) continue;
			if (!(Changed & (1u << V.Chan))) continue;
			W.Tiles.AddUnique(V.Tile);
			W.Pri = FMath::Max(W.Pri, 2);
			bAway |= !NearStar(V.Tile);
		}
		if (bAway)
		{
			// cut away to it; the star stays in the shot only when it is part of the action
			if (!bStarActs) W.Figs.Remove(Star);
			W.Mode = 2;
			W.Pri = 3;
		}
	}

	// 2. a cutaway starts a beat early, so the audience is looking before it happens, and holds a beat for the result
	for (int32 b = 0; b < N; ++b)
	{
		if (Want[b].Pri < 3) continue;
		for (const int32 n : { b - 1, b + 1 })
		{
			if (!Want.IsValidIndex(n) || Want[n].Pri > 0) continue;
			FMJWant Copy = Want[b];
			Copy.Pri = 1;
			Want[n] = MoveTemp(Copy);
		}
	}
	for (int32 b = 0; b < N; ++b)
	{
		if (Want[b].Mode != 0 && ZoomFor(Want[b], b, b) > MJMaxZoom) Want[b].Mode = 0; // too spread out for a close shot
	}
	// open on the master, so the audience knows the set before the close-ups (it shows whatever happens meanwhile)
	for (int32 b = 0; b < (N > 4 ? 2 : (N > 2 ? 1 : 0)); ++b) Want[b].Mode = 0;

	// 3. consecutive beats with the same kind of shot become one shot, as long as everything still fits in the frame
	struct FSeg { FMJWant W; int32 First = 0, Last = 0; };
	TArray<FSeg> Segs;
	for (int32 b = 0; b < N;)
	{
		FSeg S{ Want[b], b, b };
		while (S.Last + 1 < N)
		{
			const FMJWant& Next = Want[S.Last + 1];
			if (Next.Mode != S.W.Mode) break;
			if (S.W.Mode == 1 && S.Last - S.First + 1 >= 3) break; // a long walk gets a fresh angle every three beats
			if (S.W.Mode != 0)
			{
				FMJWant U = S.W;
				MJUnion(U, Next);
				if (ZoomFor(U, S.First, S.Last + 1) > MJMaxZoom) break;
				S.W = MoveTemp(U);
			}
			++S.Last;
		}
		Segs.Add(MoveTemp(S));
		b = Segs.Last().Last + 1;
	}

	// 4. one beat is too short to read: borrow a beat from a neighbour that can spare one, otherwise fold it into a
	// neighbour (two shots of a different kind fold into the master, which never leaves anything out)
	for (int32 i = 0; i < Segs.Num() && Segs.Num() > 1;)
	{
		if (Segs[i].Last > Segs[i].First) { ++i; continue; }
		bool bBorrowed = false;
		for (const int32 n : { i - 1, i + 1 })
		{
			if (!Segs.IsValidIndex(n) || Segs[n].Last - Segs[n].First < 2) continue; // three beats or more can spare one
			FSeg& X = Segs[i];
			const int32 Beat = n < i ? Segs[n].Last : Segs[n].First;
			const int32 First = FMath::Min(X.First, Beat), Last = FMath::Max(X.Last, Beat);
			FMJWant U = X.W;
			if (U.Mode != 0) MJUnion(U, Want[Beat]);
			const bool bFits = U.Mode == 0 || ZoomFor(U, First, Last) <= MJMaxZoom;
			if (!bFits && Want[Beat].Pri >= 3) continue; // the action of that beat would fall out of the frame
			if (bFits) X.W = MoveTemp(U);
			X.First = First;
			X.Last = Last;
			if (n < i) --Segs[n].Last; else ++Segs[n].First;
			bBorrowed = true;
			break;
		}
		if (bBorrowed) { ++i; continue; }
		const int32 A = i > 0 ? i - 1 : i, B = A + 1;
		FSeg M = Segs[A];
		const FSeg& Other = Segs[B];
		const bool bSameKind = M.W.Mode == Other.W.Mode;
		MJUnion(M.W, Other.W);
		M.Last = Other.Last;
		if (!bSameKind || (M.W.Mode != 0 && ZoomFor(M.W, M.First, M.Last) > MJMaxZoom)) M.W.Mode = 0;
		Segs[A] = MoveTemp(M);
		Segs.RemoveAt(B);
		i = A;
	}

	// 5. close shots alternate between the two angles, so a change of subject always reads as a cut
	int32 Angle = 1;
	for (const FSeg& S : Segs)
	{
		FMJCoverShot C;
		C.First = S.First;
		C.Last = S.Last;
		if (S.W.Mode != 0)
		{
			C.Kind = Angle;
			Angle = Angle == 1 ? 3 : 1;
			C.Follow = S.W.Figs;
			for (const FIntPoint& T : S.W.Tiles) C.Fixed.Add(TileToWorld(T));
			C.Zoom = ZoomFor(S.W, S.First, S.Last);
		}
		if (Coverage.Num() > 0 && C.Kind == 0 && Coverage.Last().Kind == 0) { Coverage.Last().Last = C.Last; continue; }
		Coverage.Add(MoveTemp(C));
	}
}

void AMJStage::UpdateCoverage(bool bEnable, float Time)
{
	int32 Want = -1;
	if (bEnable && Coverage.Num() > 0)
	{
		const int32 B = FMath::FloorToInt(Time);
		Want = Coverage.Num() - 1; // the film's last frame belongs to the last shot
		for (int32 i = 0; i < Coverage.Num(); ++i)
		{
			if (B <= Coverage[i].Last) { Want = i; break; }
		}
	}
	if (Want == Shot) return;
	const bool bWasClose = Coverage.IsValidIndex(Shot) && Coverage[Shot].Kind != 0;
	const bool bClose = Coverage.IsValidIndex(Want) && Coverage[Want].Kind != 0;
	Shot = Want;
	if (bWasClose || bClose) bShotCut = true; // master to master is not a cut
}

FString AMJStage::DescribeCoverage(const FMJLevelDef& L) const
{
	FString Out;
	for (const FMJCoverShot& C : Coverage)
	{
		if (!Out.IsEmpty()) Out += TEXT(" | ");
		Out += FString::Printf(TEXT("beats %d-%d %s"), C.First + 1, C.Last + 1, C.Kind == 0 ? TEXT("MASTER") : (C.Kind == 1 ? TEXT("LOW") : TEXT("HIGH")));
		if (C.Kind == 0) continue;
		for (int32 F : C.Follow) Out += TEXT(" ") + (L.Actors.IsValidIndex(F) ? L.Actors[F].Name : FString(TEXT("?")));
		if (C.Fixed.Num() > 0) Out += FString::Printf(TEXT(" +%d set piece%s"), C.Fixed.Num(), C.Fixed.Num() > 1 ? TEXT("s") : TEXT(""));
		if (C.Zoom > 1.01f) Out += FString::Printf(TEXT(" x%.2f"), C.Zoom);
	}
	return Out;
}

void AMJStage::SetCamMode(EMJCamMode Mode, bool bSnap)
{
	CamMode = Mode;
	CamTime = 0.f;
	if (bSnap || !bCamInit)
	{
		ComputeCamera(Mode, CamLoc, CamRot, CamFocus);
		Camera->SetRelativeLocationAndRotation(CamLoc, CamRot);
		bCamInit = true;
	}
}

void AMJStage::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector Loc; FRotator Rot; float Focus;
	ComputeCamera(CamMode, Loc, Rot, Focus);
	CamTime += DeltaSeconds;
	const bool bCoverage = CamMode == EMJCamMode::Screening && Coverage.IsValidIndex(Shot) && Coverage[Shot].Kind != 0;
	const float CoverFstop = bCoverage ? 7.f * FMath::Square(Coverage[Shot].Zoom) : 7.f; // wider coverage keeps more of the set sharp
	if (CamMode != EMJCamMode::Edit && !bCoverage)
	{
		// a slow drift so screenings and the title never look static
		Loc += FVector(0.f, FMath::Sin(CamTime * 0.33f) * (CamMode == EMJCamMode::Title ? 90.f : 55.f), 0.f);
	}
	if (bShotCut)
	{
		// a change of shot is a cut, not a move
		bShotCut = false;
		CamLoc = Loc; CamRot = Rot; CamFocus = Focus;
		Camera->PostProcessSettings.DepthOfFieldFstop = bCoverage ? CoverFstop : DofFstop * 0.7f;
	}
	const float K = 1.f - FMath::Exp(-(bCoverage ? 7.f : CamLerpSpeed) * DeltaSeconds);
	CamLoc = FMath::Lerp(CamLoc, Loc, K);
	CamRot = FMath::Lerp(CamRot, Rot, K);
	CamFocus = FMath::Lerp(CamFocus, Focus, K);
	Shake = FMath::Max(0.f, Shake - DeltaSeconds * 3.4f);
	const float ShakeT = CamTime * 60.f;
	const FVector ShakeOffset(0.f, FMath::Sin(ShakeT * 1.3f) * Shake * 16.f, FMath::Cos(ShakeT * 1.7f) * Shake * 11.f);
	Camera->SetRelativeLocationAndRotation(CamLoc + ShakeOffset, CamRot + FRotator(FMath::Sin(ShakeT * 1.1f) * Shake * 0.35f, 0.f, FMath::Cos(ShakeT * 0.9f) * Shake * 0.5f));
	Camera->SetFieldOfView(CamFov);
	Camera->PostProcessSettings.DepthOfFieldFocalDistance = CamFocus;
	// the cutting room needs the whole set readable; the title and screenings can afford a much shallower focus
	// close shots track moving figurines, so they focus on where the camera really is and stop the lens right down
	FVector Subject;
	if (bCoverage && CoverageCenter(Coverage[Shot], Subject))
	{
		CamFocus = FVector::Dist(CamLoc, Subject + FVector(0.f, 0.f, 55.f));
		Camera->PostProcessSettings.DepthOfFieldFocalDistance = CamFocus;
	}
	const float WantFstop = bCoverage ? CoverFstop : (CamMode == EMJCamMode::Edit ? DofFstop : (CamMode == EMJCamMode::Title ? DofFstop * 0.45f : DofFstop * 0.7f));
	Camera->PostProcessSettings.DepthOfFieldFstop = FMath::Lerp(Camera->PostProcessSettings.DepthOfFieldFstop, WantFstop, K);

	// idle motion that does not depend on the simulation
	const float T = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	for (FMJFixtureVis& V : Fixtures)
	{
		if ((V.Fix == EMJFix::Item || V.Fix == EMJFix::Bonus) && V.Moving)
		{
			V.Moving->SetRelativeRotation(FRotator(0.f, T * 70.f, 0.f));
			V.Moving->SetRelativeLocation(FVector(V.Fix == EMJFix::Item ? 30.f : 0.f, 0.f, (V.Fix == EMJFix::Item ? 52.f : 42.f) + FMath::Sin(T * 2.2f) * 4.f));
		}
	}
	for (FMJFigure& F : Figures)
	{
		if (F.Spinner) F.Spinner->SetRelativeRotation(FRotator(0.f, T * 220.f, 0.f));
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// posing

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
		const bool bRolls = L.Actors[i].Role == EMJRole::Crusher; // vehicles roll, they do not hop

		if (Flags & MJStep_Moved)
		{
			const float U = FMath::Clamp(A / (bRolls ? 0.95f : 0.72f), 0.f, 1.f);
			Pos = FMath::Lerp(P0, P1, bRolls ? U : Ease(U));
			if (!bRolls)
			{
				Hop = FMath::Sin(PI * U) * HopHeight;
				Squash = 1.f + 0.12f * FMath::Sin(PI * U) - (U >= 1.f ? 0.10f * FMath::Sin(PI * FMath::Clamp((A - 0.72f) / 0.28f, 0.f, 1.f)) : 0.f);
				Lean = 10.f * FMath::Sin(PI * U);
			}
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
		if (F.Spinner) F.Spinner->SetVisibility(Down > 0.9f && Death == EMJDeath::Punch, true);
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
			if (V.Moving) V.Moving->SetRelativeLocation(FVector(0.f, 0.f, -80.f * Open));
			break;
		}
		case EMJFix::Laser:
		{
			const bool bOn = !((A > 0.82f ? S1 : S0).Active & Bit);
			// an idle grid keeps short glowing stubs, so a trap that has not sprung yet can still be seen
			for (UStaticMeshComponent* Beam : V.Beams)
			{
				if (!Beam) continue;
				FVector Loc = Beam->GetRelativeLocation();
				Loc.Z = bOn ? 55.f : 10.f;
				Beam->SetRelativeLocation(Loc);
				Beam->SetRelativeScale3D(FVector(0.07f, 0.07f, bOn ? 1.0f : 0.1f));
			}
			break;
		}
		case EMJFix::Plate:
		{
			bool bPressed = false;
			for (int32 i = 0; i < Late.Actors.Num(); ++i) { if (Figures[i].bOnSet && Late.Actors[i].Pos == V.Tile) bPressed = true; }
			if (V.Moving) V.Moving->SetRelativeLocation(FVector(0.f, 0.f, bPressed ? -2.5f : 1.f));
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
		const bool bAlert = bSpottedNow && i == R.Culprit;
		for (const FIntPoint& P : Tiles)
		{
			if (Used >= SightPool.Num()) break;
			UStaticMeshComponent* Plane = SightPool[Used++];
			Plane->SetRelativeLocation(TileToWorld(P, 1.5f));
			Plane->SetMaterial(0, bAlert ? AlertEdge : SightEdge);
			Plane->SetMaterial(1, bAlert ? AlertFill : SightFill);
			Plane->SetVisibility(true);
		}
	}
	for (int32 i = Used; i < SightPool.Num(); ++i) SightPool[i]->SetVisibility(false);
}
