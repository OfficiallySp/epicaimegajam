// Fix It In Post - level definitions (authored in code as ASCII maps + strip strings).
#pragma once

#include "Sim/MJTypes.h"

/**
 * Map legend:
 *   '#' wall   '.' floor   ' ' void   '~' pit   'X' the star's mark (exit)   '*' the prize   '$' golden reel (bonus)
 *   '1'..'9'   start tile of the n-th declared actor
 *   'D' 'E' 'F' 'G' doors on channel 0..3      'd' 'e' 'f' 'g' pressure plates on channel 0..3
 *   'p' 'q' 'r' 't' levers on channel 0..3     'L' 'M' 'N' 'O' laser tiles on channel 0..3
 *
 * Strip tokens (space separated):  n e s w = move (e3 = three-beat run)   z = wait (z2 = two beats)
 *   ln le ls lw = look   u = use   a = attack   c0+ / c0- = prop switches channel 0 on / off
 *   suffix =LABEL (underscores become spaces) names the clip, suffix ! locks it.
 */
struct MEGAJAM26_API FMJLevelBuilder
{
	FMJLevelDef L;
	int32 NextUid = 0;
	FIntPoint Starts[9];
	bool bValid = true;

	FMJLevelBuilder(const TCHAR* Id, const TCHAR* Title, int32 Reel);
	FMJLevelBuilder& Text(const TCHAR* Slug, const TCHAR* Script, const TCHAR* Note = TEXT(""));
	FMJLevelBuilder& Map(std::initializer_list<const TCHAR*> Rows);
	FMJLevelBuilder& MapRows(const TArray<FString>& Rows);
	FMJLevelBuilder& Actor(EMJRole Role, const TCHAR* Name, EMJDir Facing, const TCHAR* Track, bool bLocked = false, int32 Sight = 3, const TCHAR* Look = nullptr);
	FMJLevelBuilder& Bin(int32 Actor, const TCHAR* Clips);
	FMJLevelBuilder& Rules(int32 MaxEdits, int32 Par, uint8 Tools);
	FMJLevelBuilder& NeedItem() { L.bNeedItem = true; return *this; }
	FMJLevelBuilder& Invert(uint32 Mask) { L.InvertMask = Mask; return *this; }
	FMJLevelBuilder& Bonus(EMJBonus Type, int32 Param, const TCHAR* Text, const TCHAR* Label = TEXT(""));

	static bool ParseTrack(const TCHAR* Text, TArray<FMJClip>& Out);
};

namespace MJ
{
	/** The campaign. Parsed once from the embedded level text (Sim/MJLevelData.inl). */
	MEGAJAM26_API const TArray<FMJLevelDef>& Levels();
	MEGAJAM26_API int32 FindLevel(const FString& IdOrIndex);

	/**
	 * Parse levels from text (the format is documented at the top of MJLevelData.inl).
	 * Returns the number of levels added; problems are appended to OutErrors, one per line.
	 */
	MEGAJAM26_API int32 ParseLevels(const FString& Text, TArray<FMJLevelDef>& Out, FString& OutErrors);

	/** Development: replace the campaign with the levels in a text file so designs can be iterated without compiling. */
	MEGAJAM26_API bool LoadLevelFile(const FString& Path, FString& OutErrors);
}
