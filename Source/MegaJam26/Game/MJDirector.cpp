// Fix It In Post - the game flow.
#include "Game/MJDirector.h"

#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "Game/MJStage.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Sim/MJLevels.h"
#include "UI/SMJHud.h"

DEFINE_LOG_CATEGORY_STATIC(LogMJ, Log, All);

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
		PC->bEnableClickEvents = false;
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetHideCursorDuringCapture(false);
		if (Hud.IsValid()) Mode.SetWidgetToFocus(Hud);
		PC->SetInputMode(Mode);
	}

	LoadLevel(0);
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

void AMJDirector::SetPhase(EMJPhase NewPhase)
{
	Phase = NewPhase;
	PhaseTime = 0.f;
}

void AMJDirector::Say(const FString& Text)
{
	Toast = Text;
	ToastTime = 2.2f;
}

void AMJDirector::Sfx(FName Cue)
{
	// Audio lands in a later pass; the cue names are already final so the hookup is mechanical.
	UE_LOG(LogMJ, Verbose, TEXT("sfx %s"), *Cue.ToString());
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
	SelUid = -1;
	PlayTime = 0.f;
	bFast = false;
	Resim();
	if (Stage)
	{
		// keep in step with SMJHud::ComputeLayout: toolbar + ruler + rows + padding, on a 1080-tall canvas
		const int32 Rows = Level.Actors.Num();
		Stage->PanelFraction = (66.f + 34.f + Rows * (Rows > 4 ? 62.f : 78.f) + 18.f) / 1080.f;
		Stage->Build(Level);
		Stage->SetCamMode(EMJCamMode::Edit, true);
		Stage->Apply(Level, Sim, 0.f);
	}
	SetPhase(EMJPhase::Edit);
	UE_LOG(LogMJ, Display, TEXT("Loaded level %d [%s] %s: theatrical cut -> %s"), LevelIndex, *Level.Id, *Level.Title, *MJ::OutcomeText(Level, Sim));
}

bool AMJDirector::HasNextLevel() const
{
	return LevelIndex + 1 < MJ::Levels().Num();
}

void AMJDirector::NextLevel()
{
	if (HasNextLevel()) LoadLevel(LevelIndex + 1);
	else LoadLevel(0);
}

void AMJDirector::Resim()
{
	MJ::Simulate(Level, Edit.Tracks, Sim);
	CachedCost = MJ::EditCost(Level, Edit);
	PlayTime = FMath::Clamp(PlayTime, 0.f, (float)Sim.EndBeat);
}

void AMJDirector::PushUndo()
{
	UndoStack.Add(Edit);
	if (UndoStack.Num() > 200) UndoStack.RemoveAt(0);
	RedoStack.Reset();
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
		Say(FString::Printf(TEXT("Out of edits: the studio allows %d. Undo something first."), Level.MaxEdits));
		Sfx(TEXT("denied"));
		return false;
	}
	UndoStack.Add(Before);
	RedoStack.Reset();
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
	const TCHAR* Why = !MJ::CanEditClip(Level, Before, Track, From) ? TEXT("Locked by the studio.") : TEXT("Splicing isn't unlocked yet. Cut instead.");
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
	Resim();
	Sfx(TEXT("undo"));
}

void AMJDirector::Redo()
{
	if (Phase != EMJPhase::Edit || RedoStack.Num() == 0) return;
	UndoStack.Add(Edit);
	Edit = RedoStack.Pop();
	SelUid = -1;
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

void AMJDirector::Roll()
{
	if (Phase == EMJPhase::Screening) { BackToEdit(); return; }
	Resim();
	PlayTime = 0.f;
	LastBeatFired = -1;
	EndHold = 0.f;
	SelUid = -1;
	SetPhase(EMJPhase::Screening);
	Sfx(TEXT("roll"));
}

void AMJDirector::BackToEdit()
{
	if (Phase == EMJPhase::Edit) return;
	SetPhase(EMJPhase::Edit);
	bFast = false;
}

void AMJDirector::Scrub(float Beat)
{
	if (Phase != EMJPhase::Edit) return;
	const float New = FMath::Clamp(Beat, 0.f, (float)Sim.EndBeat);
	if (FMath::FloorToInt(New) != FMath::FloorToInt(PlayTime)) Sfx(TEXT("scrub"));
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
	const FMJBeatSteps& Steps = Sim.Steps[Beat];
	uint16 All = 0;
	for (const FMJActorStep& S : Steps.Actors) All |= S.Flags;
	if (All & MJStep_Moved) Sfx(TEXT("hop"));
	if (All & MJStep_Bumped) Sfx(TEXT("bump"));
	if (All & MJStep_Hit) Sfx(TEXT("punch"));
	if (All & MJStep_Missed) Sfx(TEXT("whiff"));
	if (All & MJStep_Pulled) Sfx(TEXT("lever"));
	if (All & MJStep_Grabbed) Sfx(TEXT("grab"));
	if (All & MJStep_Switched) Sfx(TEXT("switch"));
	if (All & MJStep_Bonus) Sfx(TEXT("bonus"));
	if (All & MJStep_Spotter) Sfx(TEXT("spotted"));
	if (All & MJStep_Downed) Sfx(TEXT("down"));
	if (Sim.States[Beat].Active != Sim.States[Beat + 1].Active) Sfx(TEXT("door"));
}

void AMJDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PhaseTime += DeltaSeconds;
	if (ToastTime > 0.f) ToastTime -= DeltaSeconds;

	if (Phase == EMJPhase::Screening)
	{
		const float End = (float)Sim.EndBeat;
		if (PlayTime < End)
		{
			PlayTime = FMath::Min(End, PlayTime + DeltaSeconds * BeatsPerSecond * (bFast ? FastMultiplier : 1.f));
			const int32 Beat = FMath::Min(FMath::FloorToInt(PlayTime), Sim.EndBeat - 1);
			while (LastBeatFired < Beat) FireBeat(++LastBeatFired);
		}
		else
		{
			EndHold += DeltaSeconds;
			if (EndHold >= EndHoldSeconds)
			{
				if (Sim.IsWin())
				{
					bStarPar = CachedCost <= Level.Par;
					bStarBonus = Level.Bonus != EMJBonus::None && MJ::BonusMet(Level, Edit, Sim);
					Stars = 1 + (bStarPar ? 1 : 0) + (bStarBonus ? 1 : 0);
					SetPhase(EMJPhase::Printed);
					Sfx(TEXT("print"));
					UE_LOG(LogMJ, Display, TEXT("PRINT IT! level %s with %d edits (par %d), %d beats, stars %d"), *Level.Id, CachedCost, Level.Par, Sim.EndBeat, Stars);
				}
				else
				{
					SetPhase(EMJPhase::Failed);
					Sfx(TEXT("cutcall"));
					UE_LOG(LogMJ, Display, TEXT("CUT! level %s at beat %d: %s"), *Level.Id, Sim.EndBeat, *MJ::OutcomeText(Level, Sim));
				}
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
			RegisterDirectorCmd(TEXT("mj.Level"), TEXT("mj.Level <id|index>"), [](AMJDirector& D, const TArray<FString>& A)
			{
				const int32 I = A.Num() > 0 ? MJ::FindLevel(A[0]) : INDEX_NONE;
				if (I != INDEX_NONE) D.LoadLevel(I);
			});
			RegisterDirectorCmd(TEXT("mj.Next"), TEXT("Load the next level."), [](AMJDirector& D, const TArray<FString>&) { D.NextLevel(); });
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
			RegisterDirectorCmd(TEXT("mj.Dump"), TEXT("Log the current cut and its outcome."), [](AMJDirector& D, const TArray<FString>&)
			{
				UE_LOG(LogMJ, Display, TEXT("DUMP level=%s phase=%d cost=%d/%d play=%.2f end=%d total=%d outcome=%s | %s"),
					*D.Level.Id, (int32)D.Phase, D.Cost(), D.Level.MaxEdits, D.PlayTime, D.Sim.EndBeat, D.Sim.TotalBeats,
					*MJ::OutcomeText(D.Level, D.Sim), *MJ::DescribeTracks(D.Level, D.Edit));
			});
		}
	};
	static FMJDirectorCommands GMJDirectorCommands;
}
