// Fix It In Post - development helpers callable from editor Python.
#include "Game/MJDevLibrary.h"

#include "Sim/MJLevels.h"
#include "Sim/MJSolver.h"

namespace
{
	FString ReportLevel(const FMJLevelDef& L, bool bTrace)
	{
		FMJSolveReport R;
		const double T0 = FPlatformTime::Seconds();
		MJ::Solve(L, L.MaxEdits, R);
		FString S = FString::Printf(TEXT("%s (%.2fs)\n"), *MJ::FormatReport(L, R), FPlatformTime::Seconds() - T0);
		if (R.bTheatricalWins) S += TEXT("    !! the theatrical cut already wins\n");
		if (R.MinCost < 0) S += FString::Printf(TEXT("    !! NO SOLUTION within %d edits\n"), L.MaxEdits);
		else if (R.MinCost != L.Par) S += FString::Printf(TEXT("    !! par %d but the minimum is %d\n"), L.Par, R.MinCost);
		if (L.Bonus != EMJBonus::None && R.MinBonusCost < 0) S += TEXT("    !! bonus is impossible\n");
		for (const FString& Sample : R.Samples) S += TEXT("    ") + Sample + TEXT("\n");
		if (bTrace)
		{
			FMJEditState E;
			MJ::InitEdit(L, E);
			FMJSimResult Sim;
			MJ::Simulate(L, E.Tracks, Sim);
			S += TEXT("    theatrical: ") + MJ::DescribeTracks(L, E) + TEXT("\n") + MJ::Trace(L, Sim);
		}
		return S;
	}
}

FString UMJDevLibrary::SolveLevels(const FString& LevelText, bool bTrace)
{
	TArray<FMJLevelDef> Parsed;
	FString Errors, Out;
	MJ::ParseLevels(LevelText, Parsed, Errors);
	if (!Errors.IsEmpty()) Out += TEXT("PARSE PROBLEMS:\n") + Errors;
	for (const FMJLevelDef& L : Parsed) Out += ReportLevel(L, bTrace);
	return Out;
}

FString UMJDevLibrary::SolveCampaign()
{
	FString Out;
	for (const FMJLevelDef& L : MJ::Levels()) Out += ReportLevel(L, false);
	return Out;
}

FString UMJDevLibrary::TryEdits(const FString& LevelText, const FString& LevelId, const FString& Edits)
{
	TArray<FMJLevelDef> Parsed;
	FString Errors, Out;
	MJ::ParseLevels(LevelText, Parsed, Errors);
	if (!Errors.IsEmpty()) Out += TEXT("PARSE PROBLEMS:\n") + Errors;
	const FMJLevelDef* L = nullptr;
	for (const FMJLevelDef& Candidate : Parsed) { if (LevelId.IsEmpty() || Candidate.Id.Equals(LevelId, ESearchCase::IgnoreCase)) { L = &Candidate; break; } }
	if (!L) return Out + TEXT("level not found\n");

	FMJEditState E;
	MJ::InitEdit(*L, E);
	TArray<FString> Ops;
	Edits.ParseIntoArray(Ops, TEXT(";"));
	for (const FString& OpText : Ops)
	{
		TArray<FString> T;
		OpText.ParseIntoArrayWS(T);
		if (T.Num() == 0) continue;
		auto I = [&T](int32 Index) { return T.IsValidIndex(Index) ? FCString::Atoi(*T[Index]) : 0; };
		bool bOk = false;
		if (T[0] == TEXT("cut")) bOk = MJ::Cut(*L, E, I(1), I(2));
		else if (T[0] == TEXT("move")) bOk = MJ::Move(*L, E, I(1), I(2), I(3));
		else if (T[0] == TEXT("insert")) bOk = MJ::Insert(*L, E, I(1), I(2), I(3));
		Out += FString::Printf(TEXT("%s -> %s\n"), *OpText.TrimStartAndEnd(), bOk ? TEXT("ok") : TEXT("REFUSED"));
	}
	FMJSimResult Sim;
	MJ::Simulate(*L, E.Tracks, Sim);
	Out += FString::Printf(TEXT("cost %d/%d  end %d/%d  outcome: %s%s\n%s\n%s"), MJ::EditCost(*L, E), L->MaxEdits, Sim.EndBeat, Sim.TotalBeats,
		*MJ::OutcomeText(*L, Sim), MJ::BonusMet(*L, E, Sim) ? TEXT("  [bonus met]") : TEXT(""), *MJ::DescribeTracks(*L, E), *MJ::Trace(*L, Sim));
	return Out;
}
