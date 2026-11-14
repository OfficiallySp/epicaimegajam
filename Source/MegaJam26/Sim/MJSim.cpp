// Fix It In Post - deterministic beat simulation and edit operations.
#include "Sim/MJTypes.h"

namespace
{
	FORCEINLINE bool IsPhysical(EMJRole R) { return R != EMJRole::Prop; }

	/** Which clip (and act) is playing for a strip during beat B; Clip = -1 when the strip has run out. */
	void ClipAtBeat(const TArray<FMJClip>& Track, int32 B, int32& OutClip)
	{
		int32 T = 0;
		for (int32 i = 0; i < Track.Num(); ++i)
		{
			T += FMath::Max(1, Track[i].Beats);
			if (B < T) { OutClip = i; return; }
		}
		OutClip = -1;
	}

	void ComputeActive(const FMJLevelDef& L, FMJBeatState& S)
	{
		uint32 Plates = 0;
		for (int32 i = 0; i < S.Actors.Num(); ++i)
		{
			const FMJActorState& A = S.Actors[i];
			if (!IsPhysical(L.Actors[i].Role) || !L.InBounds(A.Pos)) continue;
			if (A.Status == EMJStatus::Dead && A.Death == EMJDeath::Fall) continue; // bodies still weigh a plate down
			const FMJCell& C = L.Cell(A.Pos);
			if (C.Fix == EMJFix::Plate) Plates |= (1u << C.Chan);
		}
		S.Active = Plates ^ S.Lever ^ S.Prop ^ L.InvertMask;
	}

	int32 LISLength(const TArray<int32, TInlineAllocator<32>>& Seq)
	{
		TArray<int32, TInlineAllocator<32>> Tails;
		for (int32 V : Seq)
		{
			int32 Lo = 0, Hi = Tails.Num();
			while (Lo < Hi) { const int32 Mid = (Lo + Hi) / 2; if (Tails[Mid] < V) Lo = Mid + 1; else Hi = Mid; }
			if (Lo == Tails.Num()) Tails.Add(V); else Tails[Lo] = V;
		}
		return Tails.Num();
	}
}

namespace MJ
{
	int32 TrackBeats(const TArray<FMJClip>& Track)
	{
		int32 T = 0;
		for (const FMJClip& C : Track) T += FMath::Max(1, C.Beats);
		return T;
	}

	FString ClipLabel(const FMJClip& C)
	{
		if (!C.Label.IsEmpty()) return C.Label;
		switch (C.Act)
		{
		case EMJAct::Wait: return TEXT("WAIT");
		case EMJAct::Move: return C.Beats > 1 ? TEXT("RUN") : TEXT("");
		case EMJAct::Look: return TEXT("LOOK");
		case EMJAct::Use: return TEXT("USE");
		case EMJAct::Attack: return TEXT("PUNCH");
		case EMJAct::Chan: return (C.Param & 0x100) ? TEXT("OFF") : TEXT("ON");
		}
		return TEXT("");
	}

	bool Blocks(const FMJLevelDef& L, const FMJBeatState& S, FIntPoint P)
	{
		if (!L.InBounds(P)) return true;
		const FMJCell& C = L.Cell(P);
		if (C.Tile == EMJTile::Wall || C.Tile == EMJTile::Void) return true;
		if (C.Fix == EMJFix::Door && !(S.Active & (1u << C.Chan))) return true;
		return false;
	}

	void SightTiles(const FMJLevelDef& L, const FMJBeatState& S, int32 Actor, TArray<FIntPoint, TInlineAllocator<8>>& Out)
	{
		Out.Reset();
		const FMJActorDef& D = L.Actors[Actor];
		const FMJActorState& A = S.Actors[Actor];
		if (A.Status != EMJStatus::Active || (D.Role != EMJRole::Guard && D.Role != EMJRole::Camera && D.Role != EMJRole::Villain)) return;
		const FIntPoint V = MJDirVec(A.Facing);
		FIntPoint P = A.Pos;
		for (int32 i = 0; i < D.Sight; ++i)
		{
			P += V;
			if (Blocks(L, S, P)) break;
			Out.Add(P);
		}
	}

	void InitEdit(const FMJLevelDef& L, FMJEditState& Out)
	{
		Out.Tracks.Reset();
		for (const FMJActorDef& A : L.Actors) Out.Tracks.Add(A.Track);
		Out.Bin = L.Bin;
	}

	void Simulate(const FMJLevelDef& L, const TArray<TArray<FMJClip>>& Tracks, FMJSimResult& Out)
	{
		const int32 NA = L.Actors.Num();
		Out = FMJSimResult();

		int32 Total = 0;
		for (const TArray<FMJClip>& T : Tracks) Total = FMath::Max(Total, TrackBeats(T));
		Out.TotalBeats = Total;

		FMJBeatState First;
		First.Actors.SetNum(NA);
		for (int32 i = 0; i < NA; ++i)
		{
			First.Actors[i].Pos = L.Actors[i].Start;
			First.Actors[i].Facing = L.Actors[i].Facing;
		}
		ComputeActive(L, First);
		Out.States.Add(First);

		const int32 Star = L.StarIndex();
		TArray<FIntPoint, TInlineAllocator<8>> Want, NewWant, Sight;
		Want.SetNum(NA);
		NewWant.SetNum(NA);

		for (int32 B = 0; B < Total; ++B)
		{
			const FMJBeatState& Prev = Out.States[B];
			FMJBeatState Next = Prev;
			FMJBeatSteps Steps;
			Steps.Actors.SetNum(NA);

			// ---- 1. intents
			for (int32 a = 0; a < NA; ++a)
			{
				FMJActorStep& St = Steps.Actors[a];
				Want[a] = Prev.Actors[a].Pos;
				int32 ClipIdx = -1;
				ClipAtBeat(Tracks[a], B, ClipIdx);
				St.Clip = (int16)ClipIdx;
				if (ClipIdx < 0 || Prev.Actors[a].Status != EMJStatus::Active) { St.Act = EMJAct::Wait; St.Clip = (Prev.Actors[a].Status != EMJStatus::Active) ? (int16)-1 : St.Clip; continue; }
				const FMJClip& C = Tracks[a][ClipIdx];
				St.Act = C.Act;
				if (C.Act == EMJAct::Move && IsPhysical(L.Actors[a].Role))
				{
					if (Next.Actors[a].Facing != C.Dir) { Next.Actors[a].Facing = C.Dir; St.Flags |= MJStep_Turned; }
					const FIntPoint T = Prev.Actors[a].Pos + MJDirVec(C.Dir);
					if (Blocks(L, Prev, T)) St.Flags |= MJStep_Bumped; else Want[a] = T;
				}
			}

			// ---- 2. simultaneous movement: resolve against a snapshot until stable
			for (int32 Guard = 0; Guard < NA + 2; ++Guard)
			{
				bool bChanged = false;
				NewWant = Want;
				for (int32 a = 0; a < NA; ++a)
				{
					const FIntPoint PA = Prev.Actors[a].Pos;
					if (Want[a] == PA) continue;
					const bool bCrushA = L.Actors[a].Role == EMJRole::Crusher;
					bool bBlock = false;
					for (int32 o = 0; o < NA && !bBlock; ++o)
					{
						if (o == a || !IsPhysical(L.Actors[o].Role) || Prev.Actors[o].Status != EMJStatus::Active) continue;
						const bool bCrushO = L.Actors[o].Role == EMJRole::Crusher;
						const FIntPoint PO = Prev.Actors[o].Pos;
						if (Want[o] == Want[a])
						{
							if (Want[o] == PO) { if (!bCrushA) bBlock = true; }            // someone is standing there
							else if (!bCrushA && !bCrushO) bBlock = true;                    // both heading for the same tile: both bump
						}
						else if (Want[o] == PA && Want[a] == PO)
						{
							if (!bCrushA && !bCrushO) bBlock = true;                         // head-on swap
						}
					}
					if (bBlock) { NewWant[a] = PA; Steps.Actors[a].Flags |= MJStep_Bumped; bChanged = true; }
				}
				Want = NewWant;
				if (!bChanged) break;
			}

			for (int32 a = 0; a < NA; ++a)
			{
				if (Want[a] != Prev.Actors[a].Pos) { Next.Actors[a].Pos = Want[a]; Steps.Actors[a].Flags |= MJStep_Moved; }
			}

			// crushers flatten whatever they drive into (or through)
			for (int32 c = 0; c < NA; ++c)
			{
				if (L.Actors[c].Role != EMJRole::Crusher || Prev.Actors[c].Status != EMJStatus::Active) continue;
				for (int32 o = 0; o < NA; ++o)
				{
					if (o == c || !IsPhysical(L.Actors[o].Role) || L.Actors[o].Role == EMJRole::Crusher || Next.Actors[o].Status == EMJStatus::Dead) continue;
					const bool bSame = Next.Actors[o].Pos == Next.Actors[c].Pos;
					const bool bSwap = Prev.Actors[o].Pos == Next.Actors[c].Pos && Next.Actors[o].Pos == Prev.Actors[c].Pos && Prev.Actors[o].Pos != Next.Actors[o].Pos;
					if (bSame || bSwap)
					{
						Next.Actors[o].Status = EMJStatus::Dead;
						Next.Actors[o].Death = EMJDeath::Crush;
						Steps.Actors[o].Flags |= MJStep_Downed;
						Steps.Actors[o].Other = (int16)c;
						Steps.Actors[c].Flags |= MJStep_Hit;
						Steps.Actors[c].Other = (int16)o;
					}
				}
			}

			// pits
			for (int32 a = 0; a < NA; ++a)
			{
				if (!IsPhysical(L.Actors[a].Role) || Next.Actors[a].Status == EMJStatus::Dead || L.Actors[a].Role == EMJRole::Crusher) continue;
				if (L.InBounds(Next.Actors[a].Pos) && L.Cell(Next.Actors[a].Pos).Tile == EMJTile::Pit && (Steps.Actors[a].Flags & MJStep_Moved))
				{
					Next.Actors[a].Status = EMJStatus::Dead;
					Next.Actors[a].Death = EMJDeath::Fall;
					Steps.Actors[a].Flags |= MJStep_Downed;
				}
			}

			// ---- 3. actions on the new positions
			for (int32 a = 0; a < NA; ++a)
			{
				FMJActorStep& St = Steps.Actors[a];
				if (St.Clip < 0 || Next.Actors[a].Status != EMJStatus::Active) continue;
				const FMJClip& C = Tracks[a][St.Clip];
				switch (C.Act)
				{
				case EMJAct::Look:
					if (Next.Actors[a].Facing != C.Dir) { Next.Actors[a].Facing = C.Dir; St.Flags |= MJStep_Turned; }
					break;
				case EMJAct::Use:
				{
					const FIntPoint P = Next.Actors[a].Pos;
					const FMJCell* Cell = L.InBounds(P) ? &L.Cell(P) : nullptr;
					if (Cell && Cell->Fix == EMJFix::Lever) { Next.Lever ^= (1u << Cell->Chan); St.Flags |= MJStep_Pulled; }
					else if (Cell && Cell->Fix == EMJFix::Item && !Next.bItemTaken) { Next.bItemTaken = true; Next.Actors[a].bHasItem = true; St.Flags |= MJStep_Grabbed; }
					else St.Flags |= MJStep_Fumbled;
					break;
				}
				case EMJAct::Attack:
				{
					const FIntPoint T = Next.Actors[a].Pos + MJDirVec(Next.Actors[a].Facing);
					bool bHit = false;
					for (int32 o = 0; o < NA; ++o)
					{
						if (o == a || Next.Actors[o].Status != EMJStatus::Active || Next.Actors[o].Pos != T) continue;
						if (L.Actors[o].Role == EMJRole::Prop || L.Actors[o].Role == EMJRole::Crusher || L.Actors[o].Role == EMJRole::Camera) continue;
						Next.Actors[o].Status = EMJStatus::KO;
						Next.Actors[o].Death = EMJDeath::Punch;
						Steps.Actors[o].Flags |= MJStep_Downed;
						Steps.Actors[o].Other = (int16)a;
						St.Other = (int16)o;
						Out.bAnyKO = true;
						bHit = true;
					}
					St.Flags |= bHit ? MJStep_Hit : MJStep_Missed;
					break;
				}
				case EMJAct::Chan:
				{
					const uint32 Bit = 1u << (C.Param & 0xFF);
					const uint32 Before = Next.Prop;
					if (C.Param & 0x100) Next.Prop |= Bit; else Next.Prop &= ~Bit;
					if (Before != Next.Prop) St.Flags |= MJStep_Switched;
					break;
				}
				default: break;
				}
			}

			// bonus reel: the star picks it up by walking over it
			if (!Next.bBonusTaken && Next.Actors[Star].Status == EMJStatus::Active && L.InBounds(Next.Actors[Star].Pos)
				&& L.Cell(Next.Actors[Star].Pos).Fix == EMJFix::Bonus)
			{
				Next.bBonusTaken = true;
				Steps.Actors[Star].Flags |= MJStep_Bonus;
			}

			// ---- 4. the world reacts
			ComputeActive(L, Next);
			for (int32 a = 0; a < NA; ++a)
			{
				if (!IsPhysical(L.Actors[a].Role) || L.Actors[a].Role == EMJRole::Crusher || L.Actors[a].Role == EMJRole::Camera) continue;
				if (Next.Actors[a].Status == EMJStatus::Dead || !L.InBounds(Next.Actors[a].Pos)) continue;
				const FMJCell& Cell = L.Cell(Next.Actors[a].Pos);
				if (Cell.Fix == EMJFix::Laser && !(Next.Active & (1u << Cell.Chan)))
				{
					Next.Actors[a].Status = EMJStatus::Dead;
					Next.Actors[a].Death = EMJDeath::Laser;
					Steps.Actors[a].Flags |= MJStep_Downed;
				}
			}

			// ---- 5. fail checks, then the win check
			bool bEnd = false;
			if (Next.Actors[Star].Status != EMJStatus::Active)
			{
				Out.Outcome = EMJOutcome::StarDown; Out.Victim = Star; Out.Culprit = Steps.Actors[Star].Other; bEnd = true;
			}
			for (int32 a = 0; a < NA && !bEnd; ++a)
			{
				if (L.Actors[a].Role == EMJRole::Ally && Next.Actors[a].Status != EMJStatus::Active)
				{
					Out.Outcome = EMJOutcome::AllyDown; Out.Victim = a; Out.Culprit = Steps.Actors[a].Other; bEnd = true;
				}
			}
			for (int32 g = 0; g < NA && !bEnd; ++g)
			{
				SightTiles(L, Next, g, Sight);
				if (Sight.Num() == 0) continue;
				for (int32 a = 0; a < NA && !bEnd; ++a)
				{
					if ((L.Actors[a].Role != EMJRole::Star && L.Actors[a].Role != EMJRole::Ally) || Next.Actors[a].Status != EMJStatus::Active) continue;
					if (Sight.Contains(Next.Actors[a].Pos))
					{
						Out.Outcome = EMJOutcome::Spotted; Out.Culprit = g; Out.Victim = a; bEnd = true;
						Steps.Actors[g].Flags |= MJStep_Spotter; Steps.Actors[g].Other = (int16)a;
						Steps.Actors[a].Flags |= MJStep_Seen;
					}
				}
			}
			if (!bEnd)
			{
				const FMJActorState& S = Next.Actors[Star];
				if (L.InBounds(S.Pos) && L.Cell(S.Pos).Fix == EMJFix::Exit && (!L.bNeedItem || S.bHasItem))
				{
					Out.Outcome = EMJOutcome::Win; bEnd = true;
					Steps.Actors[Star].Flags |= MJStep_Arrived;
				}
			}

			Out.States.Add(MoveTemp(Next));
			Out.Steps.Add(MoveTemp(Steps));
			Out.EndBeat = B + 1;
			if (bEnd) break;
		}

		Out.bBonusTaken = Out.States.Last().bBonusTaken;
	}

	int32 EditCost(const FMJLevelDef& L, const FMJEditState& E)
	{
		int32 Cost = 0;
		for (int32 t = 0; t < E.Tracks.Num() && t < L.Actors.Num(); ++t)
		{
			TArray<int32, TInlineAllocator<32>> Kept;
			int32 Inserted = 0;
			for (const FMJClip& C : E.Tracks[t]) { if (C.Orig >= 0) Kept.Add(C.Orig); else ++Inserted; }
			const int32 Deleted = L.Actors[t].Track.Num() - Kept.Num();
			Cost += Deleted + Inserted + (Kept.Num() - LISLength(Kept));
		}
		return Cost;
	}

	bool BonusMet(const FMJLevelDef& L, const FMJEditState& E, const FMJSimResult& R)
	{
		if (!R.IsWin()) return false;
		switch (L.Bonus)
		{
		case EMJBonus::None: return true;
		case EMJBonus::MaxBeats: return R.EndBeat <= L.BonusParam;
		case EMJBonus::NoKO: return !R.bAnyKO;
		case EMJBonus::Collect: return R.bBonusTaken;
		case EMJBonus::MaxEdits: return EditCost(L, E) <= L.BonusParam;
		case EMJBonus::KeepLabel:
			for (int32 t = 0; t < L.Actors.Num(); ++t)
			{
				for (const FMJClip& O : L.Actors[t].Track)
				{
					if (O.Label != L.BonusLabel) continue;
					bool bFound = false;
					for (const FMJClip& C : E.Tracks[t]) { if (C.Uid == O.Uid) { bFound = true; break; } }
					if (!bFound) return false;
				}
			}
			return true;
		}
		return false;
	}

	FString OutcomeText(const FMJLevelDef& L, const FMJSimResult& R)
	{
		auto Name = [&L](int32 i) { return L.Actors.IsValidIndex(i) ? L.Actors[i].Name : FString(TEXT("Someone")); };
		switch (R.Outcome)
		{
		case EMJOutcome::Win: return TEXT("That's the scene. Print it!");
		case EMJOutcome::Spotted: return FString::Printf(TEXT("%s was spotted by %s."), *Name(R.Victim), *Name(R.Culprit));
		case EMJOutcome::StarDown:
		case EMJOutcome::AllyDown:
		{
			const EMJDeath D = R.States.Last().Actors.IsValidIndex(R.Victim) ? R.States.Last().Actors[R.Victim].Death : EMJDeath::None;
			switch (D)
			{
			case EMJDeath::Laser: return FString::Printf(TEXT("%s walked into the lasers."), *Name(R.Victim));
			case EMJDeath::Punch: return FString::Printf(TEXT("%s was knocked out by %s."), *Name(R.Victim), *Name(R.Culprit));
			case EMJDeath::Crush: return FString::Printf(TEXT("%s was flattened by %s."), *Name(R.Victim), *Name(R.Culprit));
			case EMJDeath::Fall: return FString::Printf(TEXT("%s fell off the set."), *Name(R.Victim));
			default: return FString::Printf(TEXT("%s is down."), *Name(R.Victim));
			}
		}
		case EMJOutcome::OutOfFilm:
		default:
			if (L.bNeedItem && !R.States.Last().bItemTaken) return TEXT("The film ran out and nobody took the prize.");
			return FString::Printf(TEXT("The film ran out before %s reached the mark."), *Name(L.StarIndex()));
		}
	}

	FString DescribeTracks(const FMJLevelDef& L, const FMJEditState& E)
	{
		FString S;
		for (int32 t = 0; t < E.Tracks.Num(); ++t)
		{
			S += FString::Printf(TEXT("[%s]"), *L.Actors[t].Name);
			for (const FMJClip& C : E.Tracks[t])
			{
				static const TCHAR* DirCh[] = { TEXT("N"), TEXT("E"), TEXT("S"), TEXT("W"), TEXT("") };
				FString Lab = ClipLabel(C);
				if (C.Act == EMJAct::Move) Lab = FString(Lab.IsEmpty() ? TEXT("go") : *Lab) + DirCh[(int32)C.Dir];
				else if (C.Act == EMJAct::Look) Lab += DirCh[(int32)C.Dir];
				if (C.Beats > 1) Lab += FString::Printf(TEXT("x%d"), C.Beats);
				if (C.Orig < 0) Lab = TEXT("+") + Lab;
				S += TEXT(" ") + Lab;
			}
			S += TEXT("  ");
		}
		return S;
	}

	bool CanEditClip(const FMJLevelDef& L, const FMJEditState& E, int32 Track, int32 Index)
	{
		if (!E.Tracks.IsValidIndex(Track) || !E.Tracks[Track].IsValidIndex(Index)) return false;
		if (L.Actors[Track].bLocked) return false;
		return !E.Tracks[Track][Index].bLocked;
	}

	bool Cut(const FMJLevelDef& L, FMJEditState& E, int32 Track, int32 Index)
	{
		if (!CanEditClip(L, E, Track, Index)) return false;
		const FMJClip C = E.Tracks[Track][Index];
		if (C.Orig < 0)
		{
			// a restored deleted scene goes back to the bin
			FMJBinClip B; B.Clip = C; B.Actor = Track;
			E.Bin.Add(B);
			E.Tracks[Track].RemoveAt(Index);
			return true;
		}
		if (!(L.Tools & MJTool_Cut)) return false;
		E.Tracks[Track].RemoveAt(Index);
		return true;
	}

	bool Move(const FMJLevelDef& L, FMJEditState& E, int32 Track, int32 From, int32 To)
	{
		if (!CanEditClip(L, E, Track, From)) return false;
		TArray<FMJClip>& T = E.Tracks[Track];
		const FMJClip C = T[From];
		if (C.Orig >= 0 && !(L.Tools & MJTool_Move)) return false;
		To = FMath::Clamp(To, 0, T.Num() - 1);
		if (To == From) return false;
		T.RemoveAt(From);
		T.Insert(C, To);
		return true;
	}

	bool Insert(const FMJLevelDef& L, FMJEditState& E, int32 BinIndex, int32 Track, int32 At)
	{
		if (!(L.Tools & MJTool_Insert) || !E.Bin.IsValidIndex(BinIndex) || !E.Tracks.IsValidIndex(Track)) return false;
		if (E.Bin[BinIndex].Actor != Track || L.Actors[Track].bLocked) return false;
		const FMJClip C = E.Bin[BinIndex].Clip;
		E.Bin.RemoveAt(BinIndex);
		E.Tracks[Track].Insert(C, FMath::Clamp(At, 0, E.Tracks[Track].Num()));
		return true;
	}
}
