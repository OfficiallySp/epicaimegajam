// Fix It In Post - the game flow.
#include "Game/MJDirector.h"

#include "AudioMixerBlueprintLibrary.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "Game/MJStage.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/PackageName.h"
#include "Sim/MJLevels.h"
#include "Sim/MJSolver.h"
#include "Sound/SoundBase.h"
#include "UI/SMJHud.h"

DEFINE_LOG_CATEGORY_STATIC(LogMJ, Log, All);

namespace
{
	const TCHAR* SaveSlot = TEXT("FixItInPost");

	FString ClipName(const FMJClip& C)
	{
		static const TCHAR* DirName[] = { TEXT("NORTH"), TEXT("EAST"), TEXT("SOUTH"), TEXT("WEST"), TEXT("") };
		if (!C.Label.IsEmpty()) return C.Label;
		switch (C.Act)
		{
		case EMJAct::Move: return FString(C.Beats > 1 ? TEXT("RUN ") : TEXT("STEP ")) + DirName[(int32)C.Dir];
		case EMJAct::Look: return FString(TEXT("LOOK ")) + DirName[(int32)C.Dir];
		case EMJAct::Use: return TEXT("USE");
		case EMJAct::Attack: return TEXT("PUNCH");
		default: return TEXT("WAIT");
		}
	}

	bool SameClip(const FMJClip& A, const FMJClip& B)
	{
		return A.Act == B.Act && A.Dir == B.Dir && A.Beats == B.Beats && A.Param == B.Param && A.Label == B.Label;
	}

	bool HasUid(const TArray<FMJClip>& Track, int32 Uid)
	{
		for (const FMJClip& C : Track) { if (C.Uid == Uid) return true; }
		return false;
	}
}

AMJDirector::AMJDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

AMJDirector* AMJDirector::Get(const UObject* WorldContext)
{
	UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (!World) return nullptr;
	for (TActorIterator<AMJDirector> It(World); It; ++It) return *It;
	return nullptr;
}

void AMJDirector::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	Save = Cast<UMJSave>(UGameplayStatics::LoadGameFromSlot(SaveSlot, 0));
	if (!Save) Save = NewObject<UMJSave>(this);

	Stage = World->SpawnActor<AMJStage>(FVector::ZeroVector, FRotator::ZeroRotator);

	if (UGameViewportClient* Viewport = World->GetGameViewport())
	{
		SAssignNew(Hud, SMJHud).Director(this);
		Viewport->AddViewportWidgetContent(Hud.ToSharedRef(), 10);
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		PC->SetViewTarget(Stage);
		PC->bShowMouseCursor = true;
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetHideCursorDuringCapture(false);
		if (Hud.IsValid()) Mode.SetWidgetToFocus(Hud);
		PC->SetInputMode(Mode);
	}

	// -MJLevel=<id> jumps straight into a scene (development and capture); otherwise the title screen.
	FString StartLevel;
	if (FParse::Value(FCommandLine::Get(), TEXT("MJLevel="), StartLevel) && MJ::FindLevel(StartLevel) != INDEX_NONE)
	{
		LoadLevel(MJ::FindLevel(StartLevel));
	}
	else
	{
		GoTitle();
	}
}

void AMJDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Hud.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			if (UGameViewportClient* Viewport = World->GetGameViewport()) Viewport->RemoveViewportWidgetContent(Hud.ToSharedRef());
		}
		Hud.Reset();
	}
	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------------------------------------------------
// audio

USoundBase* AMJDirector::FindSound(FName Cue)
{
	if (TObjectPtr<USoundBase>* Found = SoundCache.Find(Cue)) return Found->Get();
	const FString Package = FString::Printf(TEXT("/Game/MegaJam26/Audio/SW_%s"), *Cue.ToString());
	USoundBase* Sound = nullptr;
	if (FPackageName::DoesPackageExist(Package))
	{
		Sound = Cast<USoundBase>(StaticLoadObject(USoundBase::StaticClass(), nullptr, *(Package + TEXT(".SW_") + Cue.ToString()), nullptr, LOAD_NoWarn | LOAD_Quiet));
	}
	SoundCache.Add(Cue, Sound); // remember misses too
	return Sound;
}

void AMJDirector::Sfx(FName Cue, float Volume, float Pitch)
{
	if (USoundBase* Sound = FindSound(Cue))
	{
		UGameplayStatics::PlaySound2D(this, Sound, Volume * (Save ? Save->SfxVolume : 1.f), Pitch);
	}
}

void AMJDirector::SetMusic(FName Cue)
{
	if (Cue == MusicCue) return;
	MusicCue = Cue;
	if (Music) { Music->FadeOut(0.8f, 0.f); Music = nullptr; }
	if (USoundBase* Sound = FindSound(Cue))
	{
		Music = UGameplayStatics::SpawnSound2D(this, Sound, 1.f, 1.f, 0.f, nullptr, false, true);
		if (Music) { Music->bIsUISound = true; Music->FadeIn(0.8f, Save ? Save->MusicVolume : 0.6f); }
	}
}

void AMJDirector::ToggleAudioCapture()
{
	if (!bCapturingAudio)
	{
		UAudioMixerBlueprintLibrary::StartRecordingOutput(this, 0.f, nullptr);
		bCapturingAudio = true;
		UE_LOG(LogMJ, Display, TEXT("Audio capture started"));
	}
	else
	{
		UAudioMixerBlueprintLibrary::StopRecordingOutput(this, EAudioRecordingExportType::WavFile, TEXT("MJCapture"), FString(), nullptr, nullptr);
		bCapturingAudio = false;
		UE_LOG(LogMJ, Display, TEXT("Audio capture written to Saved/BouncedWavFiles/MJCapture.wav"));
	}
}

void AMJDirector::AdjustVolume(bool bMusic, float Delta)
{
	if (!Save) return;
	float& V = bMusic ? Save->MusicVolume : Save->SfxVolume;
	V = FMath::Clamp(FMath::RoundToFloat((V + Delta) * 10.f) / 10.f, 0.f, 1.f);
	if (bMusic && Music) Music->SetVolumeMultiplier(FMath::Max(0.001f, V));
	if (!bMusic) Sfx(TEXT("click"));
	WriteSave();
}

// ---------------------------------------------------------------------------------------------------------------------
// flow

void AMJDirector::SetPhase(EMJPhase NewPhase)
{
	Phase = NewPhase;
	PhaseTime = 0.f;
	switch (Phase)
	{
	case EMJPhase::Title:
	case EMJPhase::Credits: SetMusic(TEXT("music_title")); break;
	case EMJPhase::Screening: SetMusic(TEXT("music_screening")); break;
	default: SetMusic(TEXT("music_edit")); break;
	}
	if (Stage)
	{
		Stage->SetCamMode(Phase == EMJPhase::Title || Phase == EMJPhase::Credits ? EMJCamMode::Title : (Phase == EMJPhase::Screening ? EMJCamMode::Screening : EMJCamMode::Edit));
	}
}

void AMJDirector::Say(const FString& Text)
{
	Toast = Text;
	ToastTime = 2.4f;
}

void AMJDirector::WriteSave()
{
	if (Save) UGameplayStatics::SaveGameToSlot(Save, SaveSlot, 0);
}

int32 AMJDirector::NumLevels() const { return MJ::Levels().Num(); }

int32 AMJDirector::StarMask(const FString& LevelId) const
{
	const int32* Found = Save ? Save->Stars.Find(LevelId) : nullptr;
	return Found ? *Found : 0;
}

int32 AMJDirector::TotalStars() const
{
	int32 N = 0;
	for (const FMJLevelDef& L : MJ::Levels()) N += FMath::CountBits((uint32)StarMask(L.Id) & (L.Bonus != EMJBonus::None ? 7u : 3u));
	return N;
}

int32 AMJDirector::MaxStars() const
{
	int32 N = 0;
	for (const FMJLevelDef& L : MJ::Levels()) N += L.Bonus != EMJBonus::None ? 3 : 2;
	return N;
}

void AMJDirector::GoTitle()
{
	Menu = EMJMenu::None;
	if (Level.Actors.Num() == 0) LoadLevel(Save ? Save->LastLevel : 0);
	bIntro = false;
	PendingSfx.Reset();
	PlayTime = 0.f;
	SetPhase(EMJPhase::Title);
}

void AMJDirector::StartGame()
{
	const TArray<FMJLevelDef>& All = MJ::Levels();
	int32 Index = 0;
	while (Index < All.Num() - 1 && (StarMask(All[Index].Id) & 1)) ++Index;
	PlayLevel(Index);
}

void AMJDirector::PlayLevel(int32 Index)
{
	LoadLevel(Index);
	bIntro = true;
	PlayTime = 0.f;
	LastBeatFired = -1;
	EndHold = 0.f;
	PendingSfx.Reset();
	SetPhase(EMJPhase::Screening);
	Sfx(TEXT("roll"));
}

void AMJDirector::LoadLevel(int32 Index)
{
	const TArray<FMJLevelDef>& All = MJ::Levels();
	if (All.Num() == 0) return;
	LevelIndex = FMath::Clamp(Index, 0, All.Num() - 1);
	Level = All[LevelIndex];
	MJ::InitEdit(Level, Edit);
	UndoStack.Reset();
	RedoStack.Reset();
	PendingSfx.Reset();
	SelUid = -1;
	HintUid = -1;
	HintTime = 0.f;
	bHintSolved = false;
	FailCount = 0;
	PlayTime = 0.f;
	bFast = false;
	bIntro = false;
	Menu = EMJMenu::None;
	Resim();
	if (Stage)
	{
		// keep in step with SMJHud::ComputeLayout: toolbar + ruler + rows + padding, on a 1080-tall canvas
		const int32 Rows = Level.Actors.Num();
		Stage->PanelFraction = (66.f + 34.f + Rows * (Rows > 4 ? 62.f : 78.f) + 18.f) / 1080.f;
		Stage->Build(Level);
		Stage->Apply(Level, Sim, 0.f);
	}
	if (Save && Save->LastLevel != LevelIndex) { Save->LastLevel = LevelIndex; WriteSave(); }
	SetPhase(EMJPhase::Edit);
	if (Stage) Stage->SetCamMode(EMJCamMode::Edit, true);
	UE_LOG(LogMJ, Display, TEXT("Loaded level %d [%s] %s: theatrical cut -> %s"), LevelIndex, *Level.Id, *Level.Title, *MJ::OutcomeText(Level, Sim));
}

bool AMJDirector::HasNextLevel() const
{
	return LevelIndex + 1 < MJ::Levels().Num();
}

void AMJDirector::NextLevel()
{
	if (HasNextLevel()) PlayLevel(LevelIndex + 1);
	else { Menu = EMJMenu::None; SetPhase(EMJPhase::Credits); }
}

void AMJDirector::OpenMenu(EMJMenu InMenu)
{
	if (Phase == EMJPhase::Screening) BackToEdit();
	Menu = InMenu;
	Sfx(TEXT("click"));
}

void AMJDirector::CloseMenu()
{
	Menu = EMJMenu::None;
}

void AMJDirector::QuitGame()
{
	WriteSave();
	UKismetSystemLibrary::QuitGame(this, GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, false);
}

void AMJDirector::ToggleFullscreen()
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		const bool bWindowed = Settings->GetFullscreenMode() == EWindowMode::Windowed;
		Settings->SetFullscreenMode(bWindowed ? EWindowMode::WindowedFullscreen : EWindowMode::Windowed);
		if (!bWindowed) Settings->SetScreenResolution(FIntPoint(1600, 900));
		else Settings->SetScreenResolution(Settings->GetDesktopResolution());
		Settings->ApplySettings(false);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// editing

void AMJDirector::Resim()
{
	MJ::Simulate(Level, Edit.Tracks, Sim);
	CachedCost = MJ::EditCost(Level, Edit);
	PlayTime = FMath::Clamp(PlayTime, 0.f, (float)Sim.EndBeat);
}

bool AMJDirector::Commit(const FMJEditState& Before, bool bChanged, const TCHAR* DeniedWhy)
{
	if (!bChanged)
	{
		Say(DeniedWhy);
		Sfx(TEXT("denied"));
		return false;
	}
	if (MJ::EditCost(Level, Edit) > Level.MaxEdits)
	{
		Edit = Before;
		Say(FString::Printf(TEXT("Out of edits: the studio allows %d here. Undo something first."), Level.MaxEdits));
		Sfx(TEXT("denied"));
		return false;
	}
	UndoStack.Add(Before);
	if (UndoStack.Num() > 200) UndoStack.RemoveAt(0);
	RedoStack.Reset();
	HintUid = -1;
	Resim();
	return true;
}

bool AMJDirector::DoCut(int32 Track, int32 Index)
{
	if (Phase != EMJPhase::Edit) return false;
	const FMJEditState Before = Edit;
	const bool bWasRestored = Edit.Tracks.IsValidIndex(Track) && Edit.Tracks[Track].IsValidIndex(Index) && Edit.Tracks[Track][Index].Orig < 0;
	const bool bOk = MJ::Cut(Level, Edit, Track, Index);
	const TCHAR* Why = !MJ::CanEditClip(Level, Before, Track, Index) ? TEXT("Locked by the studio.") : TEXT("You can't cut in this scene.");
	if (!Commit(Before, bOk, Why)) return false;
	SelUid = -1;
	Sfx(bWasRestored ? TEXT("unrestore") : TEXT("cut"));
	return true;
}

bool AMJDirector::DoMove(int32 Track, int32 From, int32 To)
{
	if (Phase != EMJPhase::Edit) return false;
	const FMJEditState Before = Edit;
	if (Edit.Tracks.IsValidIndex(Track) && FMath::Clamp(To, 0, Edit.Tracks[Track].Num() - 1) == From) return false; // dropped where it was: not an error
	const bool bOk = MJ::Move(Level, Edit, Track, From, To);
	const TCHAR* Why = !MJ::CanEditClip(Level, Before, Track, From) ? TEXT("Locked by the studio.") : TEXT("No splicing in this reel yet: you can only cut.");
	if (!Commit(Before, bOk, Why)) return false;
	Sfx(TEXT("splice"));
	return true;
}

bool AMJDirector::DoInsert(int32 BinIndex, int32 Track, int32 At)
{
	if (Phase != EMJPhase::Edit) return false;
	const FMJEditState Before = Edit;
	const bool bOk = MJ::Insert(Level, Edit, BinIndex, Track, At);
	if (!Commit(Before, bOk, TEXT("That clip belongs to a different strip."))) return false;
	Sfx(TEXT("restore"));
	return true;
}

void AMJDirector::Undo()
{
	if (Phase != EMJPhase::Edit || UndoStack.Num() == 0) return;
	RedoStack.Add(Edit);
	Edit = UndoStack.Pop();
	SelUid = -1;
	HintUid = -1;
	Resim();
	Sfx(TEXT("undo"));
}

void AMJDirector::Redo()
{
	if (Phase != EMJPhase::Edit || RedoStack.Num() == 0) return;
	UndoStack.Add(Edit);
	Edit = RedoStack.Pop();
	SelUid = -1;
	HintUid = -1;
	Resim();
	Sfx(TEXT("undo"));
}

void AMJDirector::ResetEdits()
{
	if (Phase != EMJPhase::Edit || CachedCost == 0) return;
	UndoStack.Add(Edit);
	RedoStack.Reset();
	MJ::InitEdit(Level, Edit);
	SelUid = -1;
	HintUid = -1;
	Resim();
	Sfx(TEXT("reset"));
}

bool AMJDirector::FindClip(int32 Uid, int32& OutTrack, int32& OutIndex) const
{
	for (int32 t = 0; t < Edit.Tracks.Num(); ++t)
	{
		for (int32 i = 0; i < Edit.Tracks[t].Num(); ++i)
		{
			if (Edit.Tracks[t][i].Uid == Uid) { OutTrack = t; OutIndex = i; return true; }
		}
	}
	return false;
}

void AMJDirector::Hint()
{
	if (Phase != EMJPhase::Edit) return;
	if (!bHintSolved)
	{
		FMJSolveReport Report;
		MJ::Solve(Level, Level.MaxEdits, Report, 600000);
		bHintSolved = true;
		bHintOk = Report.bHasBest;
		HintTarget = Report.Best;
	}
	Sfx(TEXT("hint"));
	HintTime = 7.f;
	HintUid = -1;
	if (!bHintOk) { HintText = TEXT("The studio has no notes on this one."); return; }
	if (Sim.IsWin()) { HintText = TEXT("This cut already works. Roll film!"); return; }

	for (int32 t = 0; t < Edit.Tracks.Num(); ++t) // 1. something in the cut that should not be there
	{
		for (const FMJClip& C : Edit.Tracks[t])
		{
			if (HasUid(HintTarget.Tracks[t], C.Uid)) continue;
			HintUid = C.Uid;
			HintText = C.Orig >= 0 ? FString::Printf(TEXT("Cut %s from %s's strip."), *ClipName(C), *Level.Actors[t].Name) : FString::Printf(TEXT("%s does not belong in %s's strip. Put it back."), *ClipName(C), *Level.Actors[t].Name);
			return;
		}
	}
	for (int32 t = 0; t < Edit.Tracks.Num(); ++t) // 2. something missing
	{
		for (const FMJClip& C : HintTarget.Tracks[t])
		{
			if (HasUid(Edit.Tracks[t], C.Uid)) continue;
			HintUid = C.Uid;
			HintText = C.Orig >= 0 ? FString::Printf(TEXT("%s should still be in %s's strip. Undo that cut."), *ClipName(C), *Level.Actors[t].Name) : FString::Printf(TEXT("Restore %s into %s's strip from the deleted scenes."), *ClipName(C), *Level.Actors[t].Name);
			return;
		}
	}
	for (int32 t = 0; t < Edit.Tracks.Num(); ++t) // 3. right clips, wrong order
	{
		for (int32 i = 0; i < Edit.Tracks[t].Num() && i < HintTarget.Tracks[t].Num(); ++i)
		{
			if (SameClip(Edit.Tracks[t][i], HintTarget.Tracks[t][i])) continue;
			const FMJClip& Want = HintTarget.Tracks[t][i];
			HintUid = Want.Uid;
			HintText = FString::Printf(TEXT("In %s's strip, %s should be clip number %d."), *Level.Actors[t].Name, *ClipName(Want), i + 1);
			return;
		}
	}
	HintText = TEXT("So close. Scrub the playhead and watch where it goes wrong.");
}

// ---------------------------------------------------------------------------------------------------------------------
// playback

void AMJDirector::Roll()
{
	if (Phase == EMJPhase::Screening) { BackToEdit(); return; }
	if (Phase != EMJPhase::Edit) return;
	Resim();
	PlayTime = 0.f;
	LastBeatFired = -1;
	EndHold = 0.f;
	SelUid = -1;
	bIntro = false;
	PendingSfx.Reset();
	SetPhase(EMJPhase::Screening);
	Sfx(TEXT("roll"));
}

void AMJDirector::BackToEdit()
{
	if (Phase == EMJPhase::Edit || Phase == EMJPhase::Title || Phase == EMJPhase::Credits) return;
	PendingSfx.Reset();
	bFast = false;
	bIntro = false;
	SetPhase(EMJPhase::Edit);
}

void AMJDirector::Scrub(float Beat)
{
	if (Phase != EMJPhase::Edit) return;
	const float New = FMath::Clamp(Beat, 0.f, (float)Sim.EndBeat);
	if (FMath::FloorToInt(New) != FMath::FloorToInt(PlayTime)) Sfx(TEXT("scrub"), 0.6f);
	PlayTime = New;
}

void AMJDirector::StepBeat(int32 Delta)
{
	if (Phase != EMJPhase::Edit) return;
	Scrub((float)(FMath::RoundToInt(PlayTime) + Delta));
}

void AMJDirector::FireBeat(int32 Beat)
{
	if (!Sim.Steps.IsValidIndex(Beat)) return;
	auto At = [this, Beat](float Offset, const TCHAR* Cue) { PendingSfx.Add({ Beat + Offset, FName(Cue) }); };
	uint16 All = 0;
	for (int32 a = 0; a < Sim.Steps[Beat].Actors.Num(); ++a)
	{
		const FMJActorStep& S = Sim.Steps[Beat].Actors[a];
		All |= S.Flags;
		if (S.Flags & MJStep_Downed)
		{
			switch (Sim.States[Beat + 1].Actors[a].Death)
			{
			case EMJDeath::Laser: At(0.78f, TEXT("zap")); break;
			case EMJDeath::Crush: At(0.45f, TEXT("crush")); break;
			case EMJDeath::Fall: At(0.50f, TEXT("fall")); break;
			default: At(0.60f, TEXT("down")); break;
			}
		}
	}
	if (All & MJStep_Moved) At(0.60f, TEXT("hop"));
	if (All & MJStep_Bumped) At(0.25f, TEXT("bump"));
	if (All & MJStep_Hit) At(0.22f, TEXT("punch"));
	if (All & MJStep_Missed) At(0.20f, TEXT("whiff"));
	if (All & MJStep_Pulled) At(0.40f, TEXT("lever"));
	if (All & MJStep_Grabbed) At(0.50f, TEXT("grab"));
	if (All & MJStep_Switched) At(0.45f, TEXT("switch"));
	if (All & MJStep_Bonus) At(0.60f, TEXT("bonus"));
	if (All & MJStep_Spotter) At(0.60f, TEXT("spotted"));
	if (Sim.States[Beat].Active != Sim.States[Beat + 1].Active) At(0.60f, TEXT("door"));
}

void AMJDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PhaseTime += DeltaSeconds;
	if (ToastTime > 0.f) ToastTime -= DeltaSeconds;
	if (HintTime > 0.f) HintTime -= DeltaSeconds;

	if (Phase == EMJPhase::Screening && Menu == EMJMenu::None)
	{
		const float End = (float)Sim.EndBeat;
		if (PlayTime < End)
		{
			const float Speed = BeatsPerSecond * (bFast ? FastMultiplier : 1.f) * (bIntro ? IntroSpeed : 1.f);
			PlayTime = FMath::Min(End, PlayTime + DeltaSeconds * Speed);
			const int32 Beat = FMath::Min(FMath::FloorToInt(PlayTime), Sim.EndBeat - 1);
			while (LastBeatFired < Beat) FireBeat(++LastBeatFired);
		}
		else
		{
			EndHold += DeltaSeconds;
		}
		for (int32 i = PendingSfx.Num() - 1; i >= 0; --i)
		{
			if (PendingSfx[i].Beat <= PlayTime) { Sfx(PendingSfx[i].Cue); PendingSfx.RemoveAtSwap(i); }
		}
		if (PlayTime >= End && EndHold >= EndHoldSeconds)
		{
			if (Sim.IsWin())
			{
				bStarPar = CachedCost <= Level.Par;
				bStarBonus = Level.Bonus != EMJBonus::None && MJ::BonusMet(Level, Edit, Sim);
				const int32 Mask = 1 | (bStarPar ? 2 : 0) | (bStarBonus ? 4 : 0);
				const int32 Old = StarMask(Level.Id);
				NewStarMask = Mask & ~Old;
				if (Save) { Save->Stars.Add(Level.Id, Old | Mask); WriteSave(); }
				SetPhase(EMJPhase::Printed);
				Sfx(TEXT("print"));
				UE_LOG(LogMJ, Display, TEXT("PRINT IT! level %s with %d edits (par %d), %d beats, star mask %d"), *Level.Id, CachedCost, Level.Par, Sim.EndBeat, Mask);
			}
			else
			{
				if (!bIntro) ++FailCount;
				SetPhase(EMJPhase::Failed);
				Sfx(TEXT("cutcall"));
				UE_LOG(LogMJ, Display, TEXT("CUT! level %s at beat %d%s: %s"), *Level.Id, Sim.EndBeat, bIntro ? TEXT(" (theatrical cut)") : TEXT(""), *MJ::OutcomeText(Level, Sim));
			}
		}
	}

	if (Stage) Stage->Apply(Level, Sim, PlayTime);
}

// ---------------------------------------------------------------------------------------------------------------------
// Console commands: every player action is scriptable so an agent (or a demo reel) can drive the game.

namespace
{
	void RegisterDirectorCmd(const TCHAR* Name, const TCHAR* Help, TFunction<void(AMJDirector&, const TArray<FString>&)> Fn)
	{
		IConsoleManager::Get().RegisterConsoleCommand(Name, Help,
			FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([Fn](const TArray<FString>& Args, UWorld* World)
			{
				if (AMJDirector* D = AMJDirector::Get(World)) Fn(*D, Args);
				else UE_LOG(LogMJ, Warning, TEXT("No director in this world (start PIE first)."));
			}), ECVF_Default);
	}

	int32 ArgInt(const TArray<FString>& Args, int32 I, int32 Default = 0)
	{
		return Args.IsValidIndex(I) ? FCString::Atoi(*Args[I]) : Default;
	}

	struct FMJDirectorCommands
	{
		FMJDirectorCommands()
		{
			RegisterDirectorCmd(TEXT("mj.Level"), TEXT("mj.Level <id|index>: load a scene into the cutting room."), [](AMJDirector& D, const TArray<FString>& A)
			{
				const int32 I = A.Num() > 0 ? MJ::FindLevel(A[0]) : INDEX_NONE;
				if (I != INDEX_NONE) D.LoadLevel(I);
			});
			RegisterDirectorCmd(TEXT("mj.Play"), TEXT("mj.Play <id|index>: open a scene the way the player sees it (theatrical cut first)."), [](AMJDirector& D, const TArray<FString>& A)
			{
				const int32 I = A.Num() > 0 ? MJ::FindLevel(A[0]) : INDEX_NONE;
				if (I != INDEX_NONE) D.PlayLevel(I);
			});
			RegisterDirectorCmd(TEXT("mj.Next"), TEXT("Load the next level."), [](AMJDirector& D, const TArray<FString>&) { D.NextLevel(); });
			RegisterDirectorCmd(TEXT("mj.Title"), TEXT("Back to the title screen."), [](AMJDirector& D, const TArray<FString>&) { D.GoTitle(); });
			RegisterDirectorCmd(TEXT("mj.Menu"), TEXT("mj.Menu <0 none|1 pause|2 scenes>"), [](AMJDirector& D, const TArray<FString>& A) { D.Menu = (EMJMenu)FMath::Clamp(ArgInt(A, 0), 0, 2); });
			RegisterDirectorCmd(TEXT("mj.Hint"), TEXT("Ask for a hint."), [](AMJDirector& D, const TArray<FString>&) { D.BackToEdit(); D.Hint(); UE_LOG(LogMJ, Display, TEXT("HINT: %s"), *D.HintText); });
			RegisterDirectorCmd(TEXT("mj.LoadFile"), TEXT("mj.LoadFile <path> [level]: replace the campaign with a level text file (development)."), [](AMJDirector& D, const TArray<FString>& A)
			{
				FString Errors;
				if (A.Num() > 0 && MJ::LoadLevelFile(A[0], Errors)) D.LoadLevel(A.Num() > 1 ? FMath::Max(0, MJ::FindLevel(A[1])) : 0);
				if (!Errors.IsEmpty()) UE_LOG(LogMJ, Warning, TEXT("mj.LoadFile: %s"), *Errors);
			});
			RegisterDirectorCmd(TEXT("mj.Cut"), TEXT("mj.Cut <track> <index>"), [](AMJDirector& D, const TArray<FString>& A) { D.BackToEdit(); D.DoCut(ArgInt(A, 0), ArgInt(A, 1)); });
			RegisterDirectorCmd(TEXT("mj.Move"), TEXT("mj.Move <track> <from> <to>"), [](AMJDirector& D, const TArray<FString>& A) { D.BackToEdit(); D.DoMove(ArgInt(A, 0), ArgInt(A, 1), ArgInt(A, 2)); });
			RegisterDirectorCmd(TEXT("mj.Insert"), TEXT("mj.Insert <bin index> <track> <at>"), [](AMJDirector& D, const TArray<FString>& A) { D.BackToEdit(); D.DoInsert(ArgInt(A, 0), ArgInt(A, 1), ArgInt(A, 2)); });
			RegisterDirectorCmd(TEXT("mj.Undo"), TEXT("Undo the last edit."), [](AMJDirector& D, const TArray<FString>&) { D.BackToEdit(); D.Undo(); });
			RegisterDirectorCmd(TEXT("mj.Redo"), TEXT("Redo."), [](AMJDirector& D, const TArray<FString>&) { D.BackToEdit(); D.Redo(); });
			RegisterDirectorCmd(TEXT("mj.Reset"), TEXT("Back to the theatrical cut."), [](AMJDirector& D, const TArray<FString>&) { D.BackToEdit(); D.ResetEdits(); });
			RegisterDirectorCmd(TEXT("mj.Roll"), TEXT("Screen the current cut."), [](AMJDirector& D, const TArray<FString>&) { D.BackToEdit(); D.Roll(); });
			RegisterDirectorCmd(TEXT("mj.Stop"), TEXT("Back to the cutting room."), [](AMJDirector& D, const TArray<FString>&) { D.BackToEdit(); });
			RegisterDirectorCmd(TEXT("mj.Scrub"), TEXT("mj.Scrub <beat>"), [](AMJDirector& D, const TArray<FString>& A) { D.BackToEdit(); D.Scrub(A.Num() > 0 ? FCString::Atof(*A[0]) : 0.f); });
			RegisterDirectorCmd(TEXT("mj.ClearSave"), TEXT("Forget all progress."), [](AMJDirector& D, const TArray<FString>&) { if (D.Save) { D.Save->Stars.Reset(); D.Save->LastLevel = 0; UGameplayStatics::SaveGameToSlot(D.Save, SaveSlot, 0); } });
			RegisterDirectorCmd(TEXT("mj.Dump"), TEXT("Log the current cut and its outcome."), [](AMJDirector& D, const TArray<FString>&)
			{
				UE_LOG(LogMJ, Display, TEXT("DUMP level=%s phase=%d menu=%d intro=%d cost=%d/%d play=%.2f end=%d total=%d stars=%d/%d outcome=%s | %s"),
					*D.Level.Id, (int32)D.Phase, (int32)D.Menu, D.bIntro ? 1 : 0, D.Cost(), D.Level.MaxEdits, D.PlayTime, D.Sim.EndBeat, D.Sim.TotalBeats,
					D.TotalStars(), D.MaxStars(), *MJ::OutcomeText(D.Level, D.Sim), *MJ::DescribeTracks(D.Level, D.Edit));
			});
		}
	};
	static FMJDirectorCommands GMJDirectorCommands;
}
