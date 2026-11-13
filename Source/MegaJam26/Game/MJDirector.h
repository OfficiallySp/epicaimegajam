// Fix It In Post - the game flow: owns the current scene, the player's cut, the simulation result, playback, save data and audio.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SaveGame.h"
#include "Sim/MJTypes.h"
#include "MJDirector.generated.h"

class AMJStage;
class SMJHud;
class UAudioComponent;
class USoundBase;

UENUM()
enum class EMJPhase : uint8
{
	Title,     // main menu over the desk
	Edit,      // the cutting room: strips are editable, the playhead can be scrubbed
	Screening, // the cut is playing (bIntro: it is the studio's theatrical cut, shown once when a scene opens)
	Failed,    // "CUT!" - the screening ended badly
	Printed,   // "PRINT IT!" - the scene works
	Credits,   // every scene printed
};

UENUM()
enum class EMJMenu : uint8 { None, Pause, Scenes };

/** Star bits per scene: 1 = printed, 2 = at or under par, 4 = bonus objective. */
UCLASS()
class MEGAJAM26_API UMJSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY() TMap<FString, int32> Stars;
	UPROPERTY() int32 LastLevel = 0;
	UPROPERTY() float MusicVolume = 0.6f;
	UPROPERTY() float SfxVolume = 0.9f;
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

	// ---- flow
	void GoTitle();
	void StartGame();                       // continue at the first scene that is not printed yet
	void PlayLevel(int32 Index);            // load a scene and show the theatrical cut first
	void LoadLevel(int32 Index);            // load a scene straight into the cutting room
	void NextLevel();
	bool HasNextLevel() const;
	void OpenMenu(EMJMenu InMenu);
	void CloseMenu();
	void QuitGame();
	void ToggleFullscreen();
	void AdjustVolume(bool bMusic, float Delta);
	int32 StarMask(const FString& LevelId) const;
	int32 TotalStars() const;
	int32 MaxStars() const;
	int32 NumLevels() const;

	// ---- editing (all return false and play a "denied" cue when not allowed)
	bool DoCut(int32 Track, int32 Index);
	bool DoMove(int32 Track, int32 From, int32 To);
	bool DoInsert(int32 BinIndex, int32 Track, int32 At);
	void Undo();
	void Redo();
	void ResetEdits();
	void Hint();
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

	void Sfx(FName Cue, float Volume = 1.f, float Pitch = 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float BeatsPerSecond = 2.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float FastMultiplier = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float IntroSpeed = 1.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MegaJam") float EndHoldSeconds = 0.45f;

	UPROPERTY() TObjectPtr<AMJStage> Stage;
	UPROPERTY() TObjectPtr<UMJSave> Save;

	EMJPhase Phase = EMJPhase::Title;
	EMJMenu Menu = EMJMenu::None;
	bool bIntro = false;    // the running / just-failed screening is the theatrical cut
	int32 LevelIndex = 0;
	FMJLevelDef Level;
	FMJEditState Edit;
	FMJSimResult Sim;
	float PlayTime = 0.f;   // in beats
	float PhaseTime = 0.f;  // seconds since the phase began
	bool bFast = false;
	int32 SelUid = -1;      // selected clip
	bool bStarPar = false, bStarBonus = false; // earned by the last print
	int32 NewStarMask = 0;  // bits earned for the first time by the last print
	FString Toast;          // short feedback line ("Out of edits")
	float ToastTime = 0.f;
	FString HintText;
	int32 HintUid = -1;
	float HintTime = 0.f;
	int32 FailCount = 0;    // failed screenings of the player's own cut in this scene

private:
	void Resim();
	bool Commit(const FMJEditState& Before, bool bChanged, const TCHAR* DeniedWhy);
	void FireBeat(int32 Beat);
	void SetPhase(EMJPhase NewPhase);
	void Say(const FString& Text);
	void SetMusic(FName Cue);
	USoundBase* FindSound(FName Cue);
	void WriteSave();

	struct FPendingSfx { float Beat; FName Cue; };
	TArray<FPendingSfx> PendingSfx;

	UPROPERTY() TMap<FName, TObjectPtr<USoundBase>> SoundCache;
	UPROPERTY() TObjectPtr<UAudioComponent> Music;
	FName MusicCue;

	TArray<FMJEditState> UndoStack, RedoStack;
	TSharedPtr<SMJHud> Hud;
	int32 CachedCost = 0;
	int32 LastBeatFired = -1;
	float EndHold = 0.f;
	bool bHintSolved = false, bHintOk = false;
	FMJEditState HintTarget;
};
