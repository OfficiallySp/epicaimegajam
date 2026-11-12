// Fix It In Post - brute-force solver used to validate level design (console: mj.Solve <id>, mj.SolveAll).
#include "Sim/MJSolver.h"
#include "Sim/MJLevels.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMJSolve, Log, All);

namespace
{
	FString Signature(const FMJEditState& E)
	{
		FString S;
		S.Reserve(64);
		for (const TArray<FMJClip>& T : E.Tracks)
		{
			for (const FMJClip& C : T) S.AppendChar((TCHAR)(C.Uid + 48));
			S.AppendChar(TEXT('|'));
		}
		return S;
	}
}

namespace MJ
{
	void Solve(const FMJLevelDef& InLevel, int32 MaxEdits, FMJSolveReport& Out, int32 MaxStates)
	{
		Out = FMJSolveReport();
		Out.WinsByCost.SetNumZeroed(MaxEdits + 1);
		Out.BonusWinsByCost.SetNumZeroed(MaxEdits + 1);

		FMJEditState Init;
		InitEdit(InLevel, Init);

		TSet<FString> Seen;
		TArray<FMJEditState> Frontier, NextFrontier;
		Frontier.Add(Init);
		Seen.Add(Signature(Init));

		FMJSimResult R;
		for (int32 Depth = 0; Depth <= MaxEdits && Frontier.Num() > 0; ++Depth)
		{
			NextFrontier.Reset();
			for (const FMJEditState& E : Frontier)
			{
				++Out.States;
				Simulate(InLevel, E.Tracks, R);
				const int32 Cost = EditCost(InLevel, E);
				if (Depth == 0) { Out.bTheatricalWins = R.IsWin(); Out.TheatricalOutcome = OutcomeText(InLevel, R); Out.TheatricalEnd = R.EndBeat; }
				if (R.IsWin() && Cost <= MaxEdits)
				{
					++Out.WinsByCost[Cost];
					if (Out.MinCost < 0 || Cost < Out.MinCost) Out.MinCost = Cost;
					const bool bBonus = BonusMet(InLevel, E, R);
					if (bBonus)
					{
						++Out.BonusWinsByCost[Cost];
						if (Out.MinBonusCost < 0 || Cost < Out.MinBonusCost) Out.MinBonusCost = Cost;
					}
					if (Out.Samples.Num() < 6) Out.Samples.Add(FString::Printf(TEXT("cost %d, %d beats%s: %s"), Cost, R.EndBeat, bBonus ? TEXT(", bonus") : TEXT(""), *DescribeTracks(InLevel, E)));
				}
				if (Depth == MaxEdits) continue;

				// expand: every legal single edit
				for (int32 t = 0; t < E.Tracks.Num(); ++t)
				{
					if (InLevel.Actors[t].bLocked) continue;
					const int32 N = E.Tracks[t].Num();
					for (int32 i = 0; i < N; ++i)
					{
						if (!CanEditClip(InLevel, E, t, i)) continue;
						if (E.Tracks[t][i].Orig >= 0 && (InLevel.Tools & MJTool_Cut))
						{
							FMJEditState C = E;
							if (Cut(InLevel, C, t, i) && EditCost(InLevel, C) <= MaxEdits)
							{
								bool bAlready = false;
								Seen.Add(Signature(C), &bAlready);
								if (!bAlready) NextFrontier.Add(MoveTemp(C));
							}
						}
						if ((InLevel.Tools & MJTool_Move) || E.Tracks[t][i].Orig < 0)
						{
							for (int32 j = 0; j < N; ++j)
							{
								if (j == i) continue;
								FMJEditState C = E;
								if (Move(InLevel, C, t, i, j) && EditCost(InLevel, C) <= MaxEdits)
								{
									bool bAlready = false;
									Seen.Add(Signature(C), &bAlready);
									if (!bAlready) NextFrontier.Add(MoveTemp(C));
								}
							}
						}
					}
					if (InLevel.Tools & MJTool_Insert)
					{
						for (int32 b = 0; b < E.Bin.Num(); ++b)
						{
							if (E.Bin[b].Actor != t) continue;
							for (int32 at = 0; at <= N; ++at)
							{
								FMJEditState C = E;
								if (Insert(InLevel, C, b, t, at) && EditCost(InLevel, C) <= MaxEdits)
								{
									bool bAlready = false;
									Seen.Add(Signature(C), &bAlready);
									if (!bAlready) NextFrontier.Add(MoveTemp(C));
								}
							}
						}
					}
				}
				if (Seen.Num() > MaxStates) { Out.bTruncated = true; break; }
			}
			if (Out.bTruncated) break;
			Swap(Frontier, NextFrontier);
		}
	}

	FString FormatReport(const FMJLevelDef& L, const FMJSolveReport& R)
	{
		FString S = FString::Printf(TEXT("[%s] %-22s theatrical=%s@%d (%s) | states=%d%s | wins by cost:"),
			*L.Id, *L.Title, R.bTheatricalWins ? TEXT("WIN(!)") : TEXT("fail"), R.TheatricalEnd, *R.TheatricalOutcome, R.States, R.bTruncated ? TEXT(" TRUNCATED") : TEXT(""));
		for (int32 c = 0; c < R.WinsByCost.Num(); ++c) S += FString::Printf(TEXT(" %d:%d"), c, R.WinsByCost[c]);
		S += FString::Printf(TEXT(" | min=%d par=%d max=%d"), R.MinCost, L.Par, L.MaxEdits);
		if (L.Bonus != EMJBonus::None)
		{
			S += TEXT(" | bonus by cost:");
			for (int32 c = 0; c < R.BonusWinsByCost.Num(); ++c) S += FString::Printf(TEXT(" %d:%d"), c, R.BonusWinsByCost[c]);
		}
		return S;
	}
}

static void MJSolveOne(int32 Index, bool bSamples)
{
	const FMJLevelDef& L = MJ::Levels()[Index];
	FMJSolveReport R;
	const double T0 = FPlatformTime::Seconds();
	MJ::Solve(L, L.MaxEdits, R);
	UE_LOG(LogMJSolve, Display, TEXT("%s (%.2fs)"), *MJ::FormatReport(L, R), FPlatformTime::Seconds() - T0);
	if (bSamples) { for (const FString& S : R.Samples) UE_LOG(LogMJSolve, Display, TEXT("    %s"), *S); }
	if (R.bTheatricalWins) { UE_LOG(LogMJSolve, Warning, TEXT("    %s: the theatrical cut already wins"), *L.Id); }
	if (R.MinCost < 0) { UE_LOG(LogMJSolve, Warning, TEXT("    %s: NO SOLUTION within %d edits"), *L.Id, L.MaxEdits); }
	else if (R.MinCost > L.Par) { UE_LOG(LogMJSolve, Warning, TEXT("    %s: par %d is below the minimum %d"), *L.Id, L.Par, R.MinCost); }
	if (L.Bonus != EMJBonus::None && R.MinBonusCost < 0) { UE_LOG(LogMJSolve, Warning, TEXT("    %s: bonus is impossible within %d edits"), *L.Id, L.MaxEdits); }
}

static FAutoConsoleCommand GMJSolveCmd(
	TEXT("mj.Solve"), TEXT("mj.Solve <level id or index>: brute-force a level and print the solution counts."),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		const int32 I = Args.Num() > 0 ? MJ::FindLevel(Args[0]) : INDEX_NONE;
		if (I == INDEX_NONE) { UE_LOG(LogMJSolve, Warning, TEXT("mj.Solve: unknown level")); return; }
		MJSolveOne(I, true);
	}));

static FAutoConsoleCommand GMJSolveAllCmd(
	TEXT("mj.SolveAll"), TEXT("Brute-force every level."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		for (int32 i = 0; i < MJ::Levels().Num(); ++i) MJSolveOne(i, false);
	}));
