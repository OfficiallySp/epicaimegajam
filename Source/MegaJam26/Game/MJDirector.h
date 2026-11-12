// Fix It In Post - the game flow: owns the current scene, the player's cut, the simulation result and playback.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sim/MJTypes.h"
#include "MJDirector.generated.h"

class AMJStage;
class SMJHud;

UENUM()
enum class EMJPhase : uint8
{
	Edit,      // the cutting room: strips are editable, the playhead can be scrubbed
	Screening, // the cut is playing
	Failed,    // "CUT!" - the screening ended badly
	Printed,   // "PRINT IT!" - the scene works
};

UCLASS()
class MEGAJAM26_API AMJDirector : public AActor
{
	GENERATED_BODY()

public:
	AMJDirector();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	static AMJDirector* Get(const UObject* WorldContext);

	// ---- level flow
	void LoadLevel(int32 Index);
	void NextLevel();
	bool HasNextLevel() const;

	// ---- editing (all return false and play a "denied" cue when not allowed)
	bool DoCut(int32 Track, int32 Index);
	bool DoMove(int32 Track, int32 From, int32 To);
	bool DoInsert(int32 BinIndex, int32 Track, int32 At);
	void Undo();
	void Redo();
	void ResetEdits();
	bool CanUndo() const { return UndoStack.Num() > 0; }
	bool CanRedo() const { return RedoStack.Num() > 0; }
	bool FindClip(int32 Uid, int32& OutTrack, int32& OutIndex) const;
	int32 Cost() const { return CachedCost; }
	int32 EditsLeft() const { return Level.MaxEdits - CachedCost; }

	// ---- playback
	void Roll();
	void BackToEdit();
	void Scrub(float Beat);
	void StepBeat(int32 Delta);

	void Sfx(FName Cue);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float BeatsPerSecond = 2.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float FastMultiplier = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float EndHoldSeconds = 0.45f;

	UPROPERTY() TObjectPtr<AMJStage> Stage;

	EMJPhase Phase = EMJPhase::Edit;
	int32 LevelIndex = 0;
	FMJLevelDef Level;
	FMJEditState Edit;
	FMJSimResult Sim;
	float PlayTime = 0.f;   // in beats
	float PhaseTime = 0.f;  // seconds since the phase began
	bool bFast = false;
	int32 SelUid = -1;      // selected clip
	int32 Stars = 0;        // stars earned by the last print
	bool bStarPar = false, bStarBonus = false;
	FString Toast;          // short feedback line ("Out of edits")
	float ToastTime = 0.f;

private:
	void Resim();
	void PushUndo();
	bool Commit(const FMJEditState& Before, bool bChanged, const TCHAR* DeniedWhy);
	void FireBeat(int32 Beat);
	void SetPhase(EMJPhase NewPhase);
	void Say(const FString& Text);

	TArray<FMJEditState> UndoStack, RedoStack;
	TSharedPtr<SMJHud> Hud;
	int32 CachedCost = 0;
	int32 LastBeatFired = -1;
	float EndHold = 0.f;
};
