// Fix It In Post - core data types for the scene simulation. Plain C++ (no UObjects) so the solver can run it millions of times.
#pragma once

#include "CoreMinimal.h"

enum class EMJDir : uint8 { N = 0, E, S, W, None };

FORCEINLINE FIntPoint MJDirVec(EMJDir D)
{
	switch (D)
	{
	case EMJDir::N: return FIntPoint(0, -1);
	case EMJDir::E: return FIntPoint(1, 0);
	case EMJDir::S: return FIntPoint(0, 1);
	case EMJDir::W: return FIntPoint(-1, 0);
	default: return FIntPoint(0, 0);
	}
}

/** What a clip makes its actor do, one beat at a time. */
enum class EMJAct : uint8
{
	Wait,   // stand still (studio filler: soda adverts, monologues...)
	Move,   // one tile per beat in Dir
	Look,   // turn to face Dir
	Use,    // pull the lever / grab the item on the actor's own tile
	Attack, // knock out whoever stands on the tile in front
	Chan,   // props only: switch a channel on (Param=1) or off (Param=0)
};

enum class EMJRole : uint8
{
	Star,    // must survive, unseen, and fulfil the script
	Ally,    // must survive, unseen
	Guard,   // sees in a straight line; can be knocked out
	Camera,  // sees in a straight line; cannot be knocked out
	Crusher, // vehicle / boulder: flattens whatever it drives into
	Prop,    // no body on the set, only a strip (laser grid controller...)
	Villain, // behaves like a guard (sight line, can be knocked out) but is billed as the villain
};

enum class EMJTile : uint8 { Void, Floor, Wall, Pit };
enum class EMJFix : uint8 { None, Door, Plate, Lever, Laser, Item, Exit, Bonus };
enum class EMJStatus : uint8 { Active, KO, Dead };
enum class EMJDeath : uint8 { None, Punch, Laser, Crush, Fall };
enum class EMJOutcome : uint8 { Win, StarDown, AllyDown, Spotted, OutOfFilm };

enum : uint8 { MJTool_Cut = 1, MJTool_Move = 2, MJTool_Insert = 4 };

/** Flags describing what one actor did during one beat (drives animation and audio). */
enum : uint16
{
	MJStep_Moved    = 1 << 0,
	MJStep_Bumped   = 1 << 1,
	MJStep_Hit      = 1 << 2, // attack connected
	MJStep_Missed   = 1 << 3, // attack hit nothing
	MJStep_Pulled   = 1 << 4, // lever
	MJStep_Grabbed  = 1 << 5, // item
	MJStep_Fumbled  = 1 << 6, // use with nothing to use
	MJStep_Spotter  = 1 << 7, // this guard saw someone
	MJStep_Seen     = 1 << 8, // this actor was seen
	MJStep_Downed   = 1 << 9, // KO'd or killed this beat
	MJStep_Turned   = 1 << 10,
	MJStep_Switched = 1 << 11, // prop flipped a channel
	MJStep_Bonus    = 1 << 12, // picked up the bonus reel
	MJStep_Arrived  = 1 << 13, // star reached the mark and the scene is printed
};

enum class EMJBonus : uint8
{
	None,
	MaxBeats,   // scene must end within Param beats
	NoKO,       // nobody gets knocked out
	Collect,    // star picks up the golden reel
	KeepLabel,  // every clip with this label is still in the cut
	MaxEdits,   // solve with at most Param edits
};

struct FMJClip
{
	EMJAct Act = EMJAct::Wait;
	EMJDir Dir = EMJDir::None;
	int32 Beats = 1;
	int32 Param = 0;      // Chan: channel index in low byte, on/off in bit 8
	bool bLocked = false; // studio-mandated: cannot be cut or moved
	int32 Uid = -1;       // unique within the level (identity for UI, undo and the solver)
	int32 Orig = -1;      // index in the theatrical track, -1 for deleted-scene clips
	FString Label;        // flavour text; empty = default for the act
};

struct FMJCell
{
	EMJTile Tile = EMJTile::Void;
	EMJFix Fix = EMJFix::None;
	int8 Chan = 0;
};

struct FMJActorDef
{
	FString Name;
	EMJRole Role = EMJRole::Guard;
	FIntPoint Start = FIntPoint(0, 0);
	EMJDir Facing = EMJDir::S;
	int32 Sight = 3;
	bool bLocked = false; // whole strip locked by the studio
	FName Look;           // which figurine to use
	TArray<FMJClip> Track; // theatrical cut
};

struct FMJBinClip
{
	FMJClip Clip;
	int32 Actor = 0; // whose strip it belongs to
};

struct FMJLevelDef
{
	FString Id;
	FString Title;
	FString Slug;   // INT. MUSEUM - NIGHT
	FString Script; // how the scene should end
	FString Note;   // studio note / tutorial hint shown in edit mode
	FString BonusText;
	int32 Reel = 1;
	int32 W = 0, H = 0;
	TArray<FMJCell> Cells;
	TArray<FMJActorDef> Actors;
	TArray<FMJBinClip> Bin;
	int32 MaxEdits = 1;
	int32 Par = 1;
	uint8 Tools = MJTool_Cut;
	bool bNeedItem = false;
	uint32 InvertMask = 0; // channels that start active (door open / laser off)
	EMJBonus Bonus = EMJBonus::None;
	int32 BonusParam = 0;
	FString BonusLabel;

	FORCEINLINE bool InBounds(FIntPoint P) const { return P.X >= 0 && P.Y >= 0 && P.X < W && P.Y < H; }
	FORCEINLINE const FMJCell& Cell(FIntPoint P) const { return Cells[P.Y * W + P.X]; }
	int32 StarIndex() const
	{
		for (int32 i = 0; i < Actors.Num(); ++i) { if (Actors[i].Role == EMJRole::Star) return i; }
		return 0;
	}
};

struct FMJActorState
{
	FIntPoint Pos = FIntPoint(0, 0);
	EMJDir Facing = EMJDir::S;
	EMJStatus Status = EMJStatus::Active;
	EMJDeath Death = EMJDeath::None;
	bool bHasItem = false;
};

struct FMJBeatState
{
	TArray<FMJActorState, TInlineAllocator<8>> Actors;
	uint32 Lever = 0;   // toggled levers per channel
	uint32 Prop = 0;    // channels forced by prop strips
	uint32 Active = 0;  // effective channel state (door open / laser off)
	bool bItemTaken = false;
	bool bBonusTaken = false;
};

struct FMJActorStep
{
	EMJAct Act = EMJAct::Wait;
	uint16 Flags = 0;
	int16 Clip = -1;   // index of the clip playing during this beat, -1 = strip finished
	int16 Other = -1;  // victim / spotter index when relevant
};

struct FMJBeatSteps
{
	TArray<FMJActorStep, TInlineAllocator<8>> Actors;
};

struct FMJSimResult
{
	TArray<FMJBeatState> States; // States[0] = start, States[b+1] = after beat b
	TArray<FMJBeatSteps> Steps;  // Steps[b] = what happened during beat b
	EMJOutcome Outcome = EMJOutcome::OutOfFilm;
	int32 EndBeat = 0;     // number of beats that actually play
	int32 TotalBeats = 0;  // length of the longest strip
	int32 Culprit = -1;    // who caused the failure (spotter, attacker...)
	int32 Victim = -1;
	bool bAnyKO = false;
	bool bBonusTaken = false;

	bool IsWin() const { return Outcome == EMJOutcome::Win; }
};

/** The player's current cut: one strip per actor plus whatever is still in the deleted-scenes bin. */
struct FMJEditState
{
	TArray<TArray<FMJClip>> Tracks;
	TArray<FMJBinClip> Bin;
};

namespace MJ
{
	MEGAJAM26_API void InitEdit(const FMJLevelDef& L, FMJEditState& Out);
	MEGAJAM26_API void Simulate(const FMJLevelDef& L, const TArray<TArray<FMJClip>>& Tracks, FMJSimResult& Out);
	MEGAJAM26_API int32 EditCost(const FMJLevelDef& L, const FMJEditState& E);
	MEGAJAM26_API bool BonusMet(const FMJLevelDef& L, const FMJEditState& E, const FMJSimResult& R);
	MEGAJAM26_API void SightTiles(const FMJLevelDef& L, const FMJBeatState& S, int32 Actor, TArray<FIntPoint, TInlineAllocator<8>>& Out);
	MEGAJAM26_API bool Blocks(const FMJLevelDef& L, const FMJBeatState& S, FIntPoint P);
	MEGAJAM26_API int32 TrackBeats(const TArray<FMJClip>& Track);
	MEGAJAM26_API FString ClipLabel(const FMJClip& C);
	MEGAJAM26_API FString OutcomeText(const FMJLevelDef& L, const FMJSimResult& R);
	MEGAJAM26_API FString DescribeTracks(const FMJLevelDef& L, const FMJEditState& E);

	/** Edit operations. Each returns false (and changes nothing) when the level's tools or locks forbid it. */
	MEGAJAM26_API bool CanEditClip(const FMJLevelDef& L, const FMJEditState& E, int32 Track, int32 Index);
	MEGAJAM26_API bool Cut(const FMJLevelDef& L, FMJEditState& E, int32 Track, int32 Index);
	MEGAJAM26_API bool Move(const FMJLevelDef& L, FMJEditState& E, int32 Track, int32 From, int32 To);
	MEGAJAM26_API bool Insert(const FMJLevelDef& L, FMJEditState& E, int32 BinIndex, int32 Track, int32 At);
}
