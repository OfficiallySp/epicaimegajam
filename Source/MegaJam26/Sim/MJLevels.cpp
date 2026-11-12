// Fix It In Post - level definitions.
#include "Sim/MJLevels.h"

namespace
{
	EMJDir DirOf(TCHAR C)
	{
		switch (C)
		{
		case 'n': return EMJDir::N;
		case 'e': return EMJDir::E;
		case 's': return EMJDir::S;
		case 'w': return EMJDir::W;
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
	L.H = (int32)Rows.size();
	L.W = 0;
	for (const TCHAR* R : Rows) L.W = FMath::Max(L.W, FCString::Strlen(R));
	L.Cells.SetNum(L.W * L.H);
	int32 Y = 0;
	for (const TCHAR* R : Rows)
	{
		const int32 Len = FCString::Strlen(R);
		for (int32 X = 0; X < L.W; ++X)
		{
			const TCHAR Ch = X < Len ? R[X] : TEXT(' ');
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
		}
		++Y;
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
	if (Look) A.Look = FName(Look);
	else
	{
		switch (Role)
		{
		case EMJRole::Star: A.Look = TEXT("Star"); break;
		case EMJRole::Ally: A.Look = TEXT("Ally"); break;
		case EMJRole::Guard: A.Look = TEXT("Guard"); break;
		case EMJRole::Camera: A.Look = TEXT("Camera"); break;
		case EMJRole::Crusher: A.Look = TEXT("Car"); break;
		default: A.Look = TEXT("Prop"); break;
		}
	}
	ParseTrack(Track, A.Track);
	for (int32 i = 0; i < A.Track.Num(); ++i) { A.Track[i].Uid = NextUid++; A.Track[i].Orig = i; }
	if (Role != EMJRole::Prop && A.Start.X < 0) UE_LOG(LogTemp, Error, TEXT("MJ: level %s actor %s has no start tile"), *L.Id, Name);
	L.Actors.Add(MoveTemp(A));
	return *this;
}

FMJLevelBuilder& FMJLevelBuilder::Bin(int32 ActorIndex, const TCHAR* Clips)
{
	TArray<FMJClip> Parsed;
	ParseTrack(Clips, Parsed);
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
	static void BuildLevels(TArray<FMJLevelDef>& Out)
	{
		const bool LOCKED = true;

		// ------------------------------------------------------------------ REEL 1: THE HEIST (cut)
		Out.Add(FMJLevelBuilder(TEXT("r1s1"), TEXT("Product Placement"), 1)
			.Text(TEXT("INT. MUSEUM CORRIDOR - NIGHT"),
				TEXT("ACE slips past the night guard and reaches his mark."),
				TEXT("The studio added a soda advert. Click a clip, then press X to cut it."))
			.Map({
				TEXT("#########"),
				TEXT("#1.....X#"),
				TEXT("#####2###"),
				TEXT("#########") })
			.Actor(EMJRole::Star, TEXT("ACE"), EMJDir::E, TEXT("e e z2=SODA_AD e e e e"))
			.Actor(EMJRole::Guard, TEXT("GUARD"), EMJDir::S, TEXT("z5=NAP ln z3"), LOCKED)
			.Rules(1, 1, MJTool_Cut)
			.L);

		Out.Add(FMJLevelBuilder(TEXT("r1s2"), TEXT("The Laser Hall"), 1)
			.Text(TEXT("INT. LASER HALL - NIGHT"),
				TEXT("ACE crosses the hall while the grid is down."),
				TEXT("Cutting a clip pulls everything after it one beat earlier."))
			.Map({
				TEXT("#########"),
				TEXT("#1..L..X#"),
				TEXT("#########") })
			.Actor(EMJRole::Star, TEXT("ACE"), EMJDir::E, TEXT("z2=SELFIE e e e e e e"))
			.Actor(EMJRole::Prop, TEXT("LASER GRID"), EMJDir::S, TEXT("z2=ON c0+=OFF z1=OFF c0-=ON z3=ON"), LOCKED)
			.Rules(1, 1, MJTool_Cut)
			.L);

		// ------------------------------------------------------------------ REEL 2: THE GETAWAY (splice)
		Out.Add(FMJLevelBuilder(TEXT("r2s1"), TEXT("Hold The Door"), 2)
			.Text(TEXT("INT. SERVICE TUNNEL - NIGHT"),
				TEXT("PIP holds the door. ACE walks through it."),
				TEXT("Drag a clip along its strip to splice it somewhere else."))
			.Map({
				TEXT("#########"),
				TEXT("#1..D..X#"),
				TEXT("#########"),
				TEXT("#2.d....#"),
				TEXT("#########") })
			.Actor(EMJRole::Star, TEXT("ACE"), EMJDir::E, TEXT("z=YAWN e e e e e e"))
			.Actor(EMJRole::Ally, TEXT("PIP"), EMJDir::E, TEXT("e e e e z2"))
			.Rules(2, 1, MJTool_Cut | MJTool_Move)
			.L);
	}

	const TArray<FMJLevelDef>& Levels()
	{
		static TArray<FMJLevelDef> All;
		if (All.Num() == 0) BuildLevels(All);
		return All;
	}

	int32 FindLevel(const FString& IdOrIndex)
	{
		const TArray<FMJLevelDef>& All = Levels();
		for (int32 i = 0; i < All.Num(); ++i) { if (All[i].Id.Equals(IdOrIndex, ESearchCase::IgnoreCase)) return i; }
		if (IdOrIndex.IsNumeric()) { const int32 I = FCString::Atoi(*IdOrIndex); if (All.IsValidIndex(I)) return I; }
		return INDEX_NONE;
	}
}
