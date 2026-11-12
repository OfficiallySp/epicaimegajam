// Fix It In Post - brute-force solver used to validate level design.
#pragma once

#include "Sim/MJTypes.h"

struct FMJSolveReport
{
	int32 States = 0;
	bool bTruncated = false;
	bool bTheatricalWins = false;
	int32 TheatricalEnd = 0;
	FString TheatricalOutcome;
	TArray<int32> WinsByCost;      // number of distinct winning cuts per edit cost
	TArray<int32> BonusWinsByCost; // ...that also meet the bonus objective
	int32 MinCost = -1;
	int32 MinBonusCost = -1;
	TArray<FString> Samples;
};

namespace MJ
{
	MEGAJAM26_API void Solve(const FMJLevelDef& L, int32 MaxEdits, FMJSolveReport& Out, int32 MaxStates = 3000000);
	MEGAJAM26_API FString FormatReport(const FMJLevelDef& L, const FMJSolveReport& R);
	MEGAJAM26_API FString Trace(const FMJLevelDef& L, const FMJSimResult& R);
}
