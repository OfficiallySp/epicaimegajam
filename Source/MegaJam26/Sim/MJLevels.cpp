// Fix It In Post - level definitions: the builder, the text parser and the embedded campaign.
#include "Sim/MJLevels.h"

#include "Misc/FileHelper.h"

namespace
{
	static const char* GMJLevelText[] = {
#include "Sim/MJLevelData.inl"
	};

	EMJDir DirOf(TCHAR C)
	{
		switch (C)
		{
		case 'n': case 'N': return EMJDir::N;
		case 'e': case 'E': return EMJDir::E;
		case 's': case 'S': return EMJDir::S;
		case 'w': case 'W': return EMJDir::W;
		default: return EMJDir::None;
		}
	}

	bool ParseClip(const FString& Token, FMJClip& C)
	{
		FString T = Token;
		if (T.EndsWith(TEXT("!"))) { C.bLocked = true; T.LeftChopInline(1); }
		int32 Eq = INDEX_NONE;
		if (T.FindChar(TEXT('='), Eq)) { C.Label = T.Mid(Eq + 1).Replace(TEXT("_"), TEXT(" ")); T.LeftInline(Eq); }
		if (T.IsEmpty()) return false;
		const TCHAR H = T[0];
		const FString Rest = T.Mid(1);
		switch (H)
		{
		case 'n': case 'e': case 's': case 'w':
			C.Act = EMJAct::Move; C.Dir = DirOf(H); C.Beats = Rest.IsEmpty() ? 1 : FMath::Max(1, FCString::Atoi(*Rest));
			return true;
		case 'z':
			C.Act = EMJAct::Wait; C.Beats = Rest.IsEmpty() ? 1 : FMath::Max(1, FCString::Atoi(*Rest));
			return true;
		case 'l':
			if (Rest.IsEmpty() || DirOf(Rest[0]) == EMJDir::None) return false;
			C.Act = EMJAct::Look; C.Dir = DirOf(Rest[0]);
			return true;
		case 'u': C.Act = EMJAct::Use; return true;
		case 'a': C.Act = EMJAct::Attack; return true;
		case 'c':
			if (Rest.Len() < 2) return false;
			C.Act = EMJAct::Chan; C.Param = (Rest[0] - TEXT('0')) | (Rest.EndsWith(TEXT("+")) ? 0x100 : 0);
			return true;
		default: return false;
		}
	}

	TArray<FMJLevelDef>& Campaign()
	{
		static TArray<FMJLevelDef> All;
		return All;
	}
}

FMJLevelBuilder::FMJLevelBuilder(const TCHAR* Id, const TCHAR* Title, int32 Reel)
{
	L.Id = Id;
	L.Title = Title;
	L.Reel = Reel;
	for (FIntPoint& P : Starts) P = FIntPoint(-1, -1);
}

FMJLevelBuilder& FMJLevelBuilder::Text(const TCHAR* Slug, const TCHAR* Script, const TCHAR* Note)
{
	L.Slug = Slug; L.Script = Script; L.Note = Note;
	return *this;
}

FMJLevelBuilder& FMJLevelBuilder::Map(std::initializer_list<const TCHAR*> Rows)
{
	TArray<FString> R;
	for (const TCHAR* Row : Rows) R.Add(Row);
	return MapRows(R);
}

FMJLevelBuilder& FMJLevelBuilder::MapRows(const TArray<FString>& Rows)
{
	L.H = Rows.Num();
	L.W = 0;
	for (const FString& R : Rows) L.W = FMath::Max(L.W, R.Len());
	L.Cells.Reset();
	L.Cells.SetNum(L.W * L.H);
	for (int32 Y = 0; Y < L.H; ++Y)
	{
		const FString& R = Rows[Y];
		for (int32 X = 0; X < L.W; ++X)
		{
			const TCHAR Ch = X < R.Len() ? R[X] : TEXT(' ');
			FMJCell& C = L.Cells[Y * L.W + X];
			C.Tile = EMJTile::Floor;
			auto Chan = [Ch](const TCHAR* Set) -> int8 { const TCHAR* F = FCString::Strchr(Set, Ch); return F ? (int8)(F - Set) : (int8)-1; };
			if (Ch == '#') C.Tile = EMJTile::Wall;
			else if (Ch == ' ') C.Tile = EMJTile::Void;
			else if (Ch == '~') C.Tile = EMJTile::Pit;
			else if (Ch == 'X') C.Fix = EMJFix::Exit;
			else if (Ch == '*') C.Fix = EMJFix::Item;
			else if (Ch == '$') C.Fix = EMJFix::Bonus;
			else if (Ch >= '1' && Ch <= '9') Starts[Ch - '1'] = FIntPoint(X, Y);
			else if (Chan(TEXT("DEFG")) >= 0) { C.Fix = EMJFix::Door; C.Chan = Chan(TEXT("DEFG")); }
			else if (Chan(TEXT("defg")) >= 0) { C.Fix = EMJFix::Plate; C.Chan = Chan(TEXT("defg")); }
			else if (Chan(TEXT("pqrt")) >= 0) { C.Fix = EMJFix::Lever; C.Chan = Chan(TEXT("pqrt")); }
			else if (Chan(TEXT("LMNO")) >= 0) { C.Fix = EMJFix::Laser; C.Chan = Chan(TEXT("LMNO")); }
			else if (Ch != '.') UE_LOG(LogTemp, Warning, TEXT("MJ: level %s has an unknown map character '%c'"), *L.Id, Ch);
		}
	}
	return *this;
}

bool FMJLevelBuilder::ParseTrack(const TCHAR* Text, TArray<FMJClip>& Out)
{
	TArray<FString> Tokens;
	FString(Text).ParseIntoArrayWS(Tokens);
	bool bOk = true;
	for (const FString& Tok : Tokens)
	{
		FMJClip C;
		if (ParseClip(Tok, C)) Out.Add(C);
		else { bOk = false; UE_LOG(LogTemp, Error, TEXT("MJ: bad clip token '%s'"), *Tok); }
	}
	return bOk;
}

FMJLevelBuilder& FMJLevelBuilder::Actor(EMJRole Role, const TCHAR* Name, EMJDir Facing, const TCHAR* Track, bool bLocked, int32 Sight, const TCHAR* Look)
{
	FMJActorDef A;
	A.Name = Name;
	A.Role = Role;
	A.Facing = Facing;
	A.bLocked = bLocked;
	A.Sight = Sight;
	const int32 Index = L.Actors.Num();
	A.Start = Index < 9 ? Starts[Index] : FIntPoint(-1, -1);
	if (Look && *Look) A.Look = FName(Look);
	else
	{
		switch (Role)
		{
		case EMJRole::Star: A.Look = TEXT("Star"); break;
		case EMJRole::Ally: A.Look = TEXT("Ally"); break;
		case EMJRole::Guard: A.Look = TEXT("Guard"); break;
		case EMJRole::Camera: A.Look = TEXT("Camera"); break;
		case EMJRole::Crusher: A.Look = TEXT("Car"); break;
		case EMJRole::Villain: A.Look = TEXT("Villain"); break;
		default: A.Look = TEXT("Prop"); break;
		}
	}
	bValid &= ParseTrack(Track, A.Track);
	for (int32 i = 0; i < A.Track.Num(); ++i) { A.Track[i].Uid = NextUid++; A.Track[i].Orig = i; }
	if (Role != EMJRole::Prop && A.Start.X < 0) { bValid = false; UE_LOG(LogTemp, Error, TEXT("MJ: level %s actor %s has no start tile"), *L.Id, Name); }
	L.Actors.Add(MoveTemp(A));
	return *this;
}

FMJLevelBuilder& FMJLevelBuilder::Bin(int32 ActorIndex, const TCHAR* Clips)
{
	TArray<FMJClip> Parsed;
	bValid &= ParseTrack(Clips, Parsed);
	for (FMJClip& C : Parsed)
	{
		C.Uid = NextUid++;
		C.Orig = -1;
		FMJBinClip B; B.Clip = C; B.Actor = ActorIndex;
		L.Bin.Add(B);
	}
	return *this;
}

FMJLevelBuilder& FMJLevelBuilder::Rules(int32 MaxEdits, int32 Par, uint8 Tools)
{
	L.MaxEdits = MaxEdits; L.Par = Par; L.Tools = Tools;
	return *this;
}

FMJLevelBuilder& FMJLevelBuilder::Bonus(EMJBonus Type, int32 Param, const TCHAR* InText, const TCHAR* Label)
{
	L.Bonus = Type; L.BonusParam = Param; L.BonusText = InText; L.BonusLabel = Label;
	return *this;
}

namespace MJ
{
	int32 ParseLevels(const FString& Text, TArray<FMJLevelDef>& Out, FString& OutErrors)
	{
		TArray<FString> Lines;
		Text.ParseIntoArrayLines(Lines, false);
		TUniquePtr<FMJLevelBuilder> B;
		TArray<FString> Rows;
		bool bInMap = false;
		int32 Added = 0;

		auto Err = [&OutErrors](int32 LineNo, const FString& Msg) { OutErrors += FString::Printf(TEXT("line %d: %s\n"), LineNo + 1, *Msg); };
		auto Finish = [&]()
		{
			if (!B) return;
			FMJLevelDef& L = B->L;
			bool bOk = B->bValid;
			if (L.W == 0) { OutErrors += FString::Printf(TEXT("level %s: no map\n"), *L.Id); bOk = false; }
			bool bStar = false, bExit = false;
			for (const FMJActorDef& A : L.Actors) bStar |= A.Role == EMJRole::Star;
			for (const FMJCell& C : L.Cells) bExit |= C.Fix == EMJFix::Exit;
			if (!bStar) { OutErrors += FString::Printf(TEXT("level %s: no star\n"), *L.Id); bOk = false; }
			if (!bExit) { OutErrors += FString::Printf(TEXT("level %s: no mark (X)\n"), *L.Id); bOk = false; }
			for (const FMJBinClip& C : L.Bin) { if (!L.Actors.IsValidIndex(C.Actor)) { OutErrors += FString::Printf(TEXT("level %s: bin clip for missing actor %d\n"), *L.Id, C.Actor); bOk = false; } }
			if (!B->bValid) OutErrors += FString::Printf(TEXT("level %s: bad strip or actor (see log)\n"), *L.Id);
			if (bOk) { Out.Add(L); ++Added; }
			B.Reset();
		};

		for (int32 i = 0; i < Lines.Num(); ++i)
		{
			if (bInMap)
			{
				if (Lines[i].TrimStartAndEnd().Equals(TEXT("endmap"), ESearchCase::IgnoreCase))
				{
					bInMap = false;
					if (B) B->MapRows(Rows);
					Rows.Reset();
				}
				else Rows.Add(Lines[i].TrimEnd());
				continue;
			}
			const FString Line = Lines[i].TrimStartAndEnd();
			if (Line.IsEmpty() || Line.StartsWith(TEXT("//"))) continue;
			FString Key, Rest;
			if (!Line.Split(TEXT(" "), &Key, &Rest)) { Key = Line; Rest.Reset(); }
			Rest.TrimStartAndEndInline();
			Key.ToLowerInline();

			if (Key == TEXT("level")) { Finish(); B = MakeUnique<FMJLevelBuilder>(*Rest, TEXT(""), 1); continue; }
			if (Key == TEXT("end")) { Finish(); continue; }
			if (!B) { Err(i, TEXT("expected 'level <id>'")); continue; }
			FMJLevelDef& L = B->L;

			if (Key == TEXT("title")) L.Title = Rest;
			else if (Key == TEXT("slug")) L.Slug = Rest;
			else if (Key == TEXT("script")) L.Script = Rest;
			else if (Key == TEXT("note")) L.Note = Rest;
			else if (Key == TEXT("reel")) L.Reel = FMath::Clamp(FCString::Atoi(*Rest), 1, 3);
			else if (Key == TEXT("map")) bInMap = true;
			else if (Key == TEXT("needitem")) L.bNeedItem = true;
			else if (Key == TEXT("invert"))
			{
				TArray<FString> Parts;
				Rest.ParseIntoArray(Parts, TEXT(","));
				for (const FString& P : Parts) L.InvertMask |= 1u << FMath::Clamp(FCString::Atoi(*P), 0, 31);
			}
			else if (Key == TEXT("actor"))
			{
				FString Head, Track;
				if (!Rest.Split(TEXT("|"), &Head, &Track)) { Head = Rest; }
				TArray<FString> T;
				Head.ParseIntoArrayWS(T);
				if (T.Num() < 3) { Err(i, TEXT("actor needs: <role> <NAME> <facing> [options] | <strip>")); continue; }
				EMJRole Role = EMJRole::Guard;
				const FString R = T[0].ToLower();
				if (R == TEXT("star")) Role = EMJRole::Star;
				else if (R == TEXT("ally")) Role = EMJRole::Ally;
				else if (R == TEXT("guard")) Role = EMJRole::Guard;
				else if (R == TEXT("camera")) Role = EMJRole::Camera;
				else if (R == TEXT("crusher")) Role = EMJRole::Crusher;
				else if (R == TEXT("prop")) Role = EMJRole::Prop;
				else if (R == TEXT("villain")) Role = EMJRole::Villain;
				else Err(i, FString::Printf(TEXT("unknown role '%s'"), *T[0]));
				const EMJDir Facing = DirOf(T[2][0]);
				if (Facing == EMJDir::None) Err(i, FString::Printf(TEXT("bad facing '%s'"), *T[2]));
				bool bLocked = false;
				int32 Sight = 3;
				FString Look;
				for (int32 j = 3; j < T.Num(); ++j)
				{
					if (T[j].Equals(TEXT("locked"), ESearchCase::IgnoreCase)) bLocked = true;
					else if (T[j].StartsWith(TEXT("sight="))) Sight = FCString::Atoi(*T[j].Mid(6));
					else if (T[j].StartsWith(TEXT("look="))) Look = T[j].Mid(5);
					else Err(i, FString::Printf(TEXT("unknown actor option '%s'"), *T[j]));
				}
				B->Actor(Role, *T[1].Replace(TEXT("_"), TEXT(" ")), Facing == EMJDir::None ? EMJDir::S : Facing, *Track, bLocked, Sight, *Look);
			}
			else if (Key == TEXT("bin"))
			{
				FString Head, Clips;
				if (!Rest.Split(TEXT("|"), &Head, &Clips)) { Err(i, TEXT("bin needs: <actor index> | <clips>")); continue; }
				B->Bin(FCString::Atoi(*Head.TrimStartAndEnd()), *Clips);
			}
			else if (Key == TEXT("rules"))
			{
				TArray<FString> T;
				Rest.ParseIntoArrayWS(T);
				for (const FString& Opt : T)
				{
					if (Opt.StartsWith(TEXT("edits="))) L.MaxEdits = FCString::Atoi(*Opt.Mid(6));
					else if (Opt.StartsWith(TEXT("par="))) L.Par = FCString::Atoi(*Opt.Mid(4));
					else if (Opt.StartsWith(TEXT("hints=")))
					{
						const FString V = Opt.Mid(6).ToLower();
						if (V == TEXT("full")) L.Hints = 0;
						else if (V == TEXT("vague")) L.Hints = 1;
						else if (V == TEXT("none")) L.Hints = 2;
						else Err(i, FString::Printf(TEXT("hints must be full, vague or none, not '%s'"), *V));
					}
					else if (Opt.StartsWith(TEXT("tools=")))
					{
						L.Tools = 0;
						const FString V = Opt.Mid(6).ToLower();
						if (V.Contains(TEXT("cut"))) L.Tools |= MJTool_Cut;
						if (V.Contains(TEXT("move"))) L.Tools |= MJTool_Move;
						if (V.Contains(TEXT("insert"))) L.Tools |= MJTool_Insert;
					}
					else Err(i, FString::Printf(TEXT("unknown rule '%s'"), *Opt));
				}
			}
			else if (Key == TEXT("bonus"))
			{
				FString Head, Shown;
				if (!Rest.Split(TEXT("|"), &Head, &Shown)) { Head = Rest; }
				TArray<FString> T;
				Head.ParseIntoArrayWS(T);
				if (T.Num() == 0) { Err(i, TEXT("bonus needs a type")); continue; }
				const FString Type = T[0].ToLower();
				const int32 Param = T.Num() > 1 ? FCString::Atoi(*T[1]) : 0;
				L.BonusText = Shown.TrimStartAndEnd();
				L.BonusParam = Param;
				if (Type == TEXT("maxbeats")) L.Bonus = EMJBonus::MaxBeats;
				else if (Type == TEXT("maxedits")) L.Bonus = EMJBonus::MaxEdits;
				else if (Type == TEXT("noko")) L.Bonus = EMJBonus::NoKO;
				else if (Type == TEXT("collect")) L.Bonus = EMJBonus::Collect;
				else if (Type == TEXT("keep")) { L.Bonus = EMJBonus::KeepLabel; L.BonusLabel = T.Num() > 1 ? T[1].Replace(TEXT("_"), TEXT(" ")) : FString(); }
				else Err(i, FString::Printf(TEXT("unknown bonus '%s'"), *T[0]));
			}
			else Err(i, FString::Printf(TEXT("unknown keyword '%s'"), *Key));
		}
		Finish();
		return Added;
	}

	const TArray<FMJLevelDef>& Levels()
	{
		TArray<FMJLevelDef>& All = Campaign();
		if (All.Num() == 0)
		{
			FString Errors;
			for (const char* Chunk : GMJLevelText) ParseLevels(FString(UTF8_TO_TCHAR(Chunk)), All, Errors);
			if (!Errors.IsEmpty()) UE_LOG(LogTemp, Error, TEXT("MJ: embedded level data has problems:\n%s"), *Errors);
		}
		return All;
	}

	bool LoadLevelFile(const FString& Path, FString& OutErrors)
	{
		FString Text;
		if (!FFileHelper::LoadFileToString(Text, *Path)) { OutErrors = FString::Printf(TEXT("cannot read %s"), *Path); return false; }
		TArray<FMJLevelDef> Parsed;
		ParseLevels(Text, Parsed, OutErrors);
		if (Parsed.Num() == 0) return false;
		Campaign() = MoveTemp(Parsed);
		return true;
	}

	int32 FindLevel(const FString& IdOrIndex)
	{
		const TArray<FMJLevelDef>& All = Levels();
		for (int32 i = 0; i < All.Num(); ++i) { if (All[i].Id.Equals(IdOrIndex, ESearchCase::IgnoreCase)) return i; }
		if (IdOrIndex.IsNumeric()) { const int32 I = FCString::Atoi(*IdOrIndex); if (All.IsValidIndex(I)) return I; }
		return INDEX_NONE;
	}
}
