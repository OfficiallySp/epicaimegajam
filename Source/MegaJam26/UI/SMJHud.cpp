// Fix It In Post - in-game UI. One leaf widget paints everything and does its own hit testing; the director holds all state.
#include "UI/SMJHud.h"

#include "Brushes/SlateRoundedBoxBrush.h"
#include "Fonts/CompositeFont.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Game/MJDirector.h"
#include "UnrealClient.h"
#include "Misc/Paths.h"
#include "Rendering/DrawElements.h"
#include "Rendering/RenderingCommon.h"
#include "Rendering/SlateRenderer.h"
#include "Rendering/SlateResourceHandle.h"
#include "Sim/MJLevels.h"
#include "Textures/SlateShaderResource.h"
#include "Sim/MJTypes.h"
#include "Styling/CoreStyle.h"

namespace
{
	FLinearColor Hex(uint32 RGB, float A = 1.f)
	{
		FLinearColor C = FLinearColor::FromSRGBColor(FColor((RGB >> 16) & 0xFF, (RGB >> 8) & 0xFF, RGB & 0xFF));
		C.A = A;
		return C;
	}

	FLinearColor Fade(const FLinearColor& C, float A) { return FLinearColor(C.R, C.G, C.B, C.A * A); }
	FLinearColor Shade(const FLinearColor& C, float K) { return FLinearColor(C.R * K, C.G * K, C.B * K, C.A); }

	const FLinearColor ColPanel = Hex(0x101218, 0.96f);
	const FLinearColor ColStrip = Hex(0x07080B);
	const FLinearColor ColHole = Hex(0x2B303C);
	const FLinearColor ColText = Hex(0xF2E9D8);
	const FLinearColor ColDim = Hex(0x8F98AB);
	const FLinearColor ColAmber = Hex(0xFFB020);
	const FLinearColor ColRed = Hex(0xE5484D);
	const FLinearColor ColGreen = Hex(0x3DD68C);
	const FLinearColor ColPaper = Hex(0xEFE6D2);
	const FLinearColor ColInk = Hex(0x1B1A17);

	FLinearColor ActColor(const FMJClip& C)
	{
		switch (C.Act)
		{
		case EMJAct::Move: return Hex(0x2F7DD1);
		case EMJAct::Wait: return Hex(0xB98A1E);
		case EMJAct::Look: return Hex(0x8562D6);
		case EMJAct::Use: return Hex(0x24A070);
		case EMJAct::Attack: return Hex(0xD4513B);
		case EMJAct::Chan: return (C.Param & 0x100) ? Hex(0x3E8E5A) : Hex(0xC73E3E);
		}
		return Hex(0x666666);
	}

	FLinearColor RoleColor(EMJRole R)
	{
		switch (R)
		{
		case EMJRole::Star: return Hex(0xFFC53D);
		case EMJRole::Ally: return Hex(0x2CC5D2);
		case EMJRole::Guard: return Hex(0xE5484D);
		case EMJRole::Camera: return Hex(0xB0B4BF);
		case EMJRole::Crusher: return Hex(0xFF7A45);
		case EMJRole::Villain: return Hex(0xD9365E);
		default: return Hex(0x9AA3B5);
		}
	}

	const TCHAR* RoleText(EMJRole R)
	{
		switch (R)
		{
		case EMJRole::Star: return TEXT("THE STAR");
		case EMJRole::Ally: return TEXT("SIDEKICK");
		case EMJRole::Guard: return TEXT("GUARD");
		case EMJRole::Camera: return TEXT("CAMERA");
		case EMJRole::Crusher: return TEXT("VEHICLE");
		case EMJRole::Villain: return TEXT("THE VILLAIN");
		default: return TEXT("SET");
		}
	}

	const FSlateBrush* WhiteBrush()
	{
		static const FSlateBrush* B = FCoreStyle::Get().GetBrush("WhiteBrush");
		return B;
	}

	const FSlateBrush* RoundBrush(float Radius)
	{
		static TMap<int32, TSharedPtr<FSlateRoundedBoxBrush>> Cache;
		const int32 Key = FMath::Clamp(FMath::RoundToInt(Radius), 1, 200);
		TSharedPtr<FSlateRoundedBoxBrush>& B = Cache.FindOrAdd(Key);
		if (!B.IsValid()) B = MakeShared<FSlateRoundedBoxBrush>(FLinearColor::White, (float)Key);
		return B.Get();
	}

	/**
	 * Outline-only rounded box. MakeBox's tint only colours the fill, so the outline colour has to live in the brush:
	 * one cached brush per (radius, thickness, colour), drawn with a transparent tint.
	 */
	const FSlateBrush* FrameBrush(float Radius, float Thick, const FLinearColor& Color)
	{
		static TMap<uint64, TSharedPtr<FSlateRoundedBoxBrush>> Cache;
		const uint64 R = (uint64)FMath::Clamp(FMath::RoundToInt(Radius), 0, 255);
		const uint64 T = (uint64)FMath::Clamp(FMath::RoundToInt(Thick * 2.f), 1, 63);
		FColor Q = Color.ToFColor(true);
		Q.A = (uint8)(FMath::RoundToInt(Q.A / 17.f) * 17); // 16 alpha steps keep fades from minting a brush per frame
		const uint64 Key = (R << 40) | (T << 32) | (uint64)Q.ToPackedARGB();
		TSharedPtr<FSlateRoundedBoxBrush>& B = Cache.FindOrAdd(Key);
		if (!B.IsValid()) B = MakeShared<FSlateRoundedBoxBrush>(FLinearColor::Transparent, (float)R, FLinearColor(Q), T * 0.5f);
		return B.Get();
	}
}

/** Fonts shipped as loose files in Content/Fonts (staged with the build); null when the file is missing. */
static TSharedPtr<const FCompositeFont> FileFont(const TCHAR* File)
{
	struct FEntry { bool bTried = false; TSharedPtr<const FCompositeFont> Font; };
	static TMap<FString, FEntry> Cache;
	FEntry& E = Cache.FindOrAdd(File);
	if (!E.bTried)
	{
		E.bTried = true;
		const FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("Fonts") / File);
		if (FPaths::FileExists(Path)) E.Font = MakeShared<FStandaloneCompositeFont>(FName(TEXT("Regular")), Path, EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
	}
	return E.Font;
}

/** Draw helper working in virtual (1080-tall) coordinates. */
struct FMJPaint
{
	const FGeometry& G;
	FSlateWindowElementList& Out;
	int32 Layer;
	float S;

	FPaintGeometry Geo(FVector2f P, FVector2f Sz) const
	{
		return G.ToPaintGeometry(Sz * S, FSlateLayoutTransform(P * S));
	}

	void Box(FVector2f P, FVector2f Sz, const FLinearColor& C, float Radius = 0.f) const
	{
		if (Sz.X <= 0.f || Sz.Y <= 0.f || C.A <= 0.f) return;
		FSlateDrawElement::MakeBox(Out, Layer, Geo(P, Sz), Radius > 0.f ? RoundBrush(Radius * S) : WhiteBrush(), ESlateDrawEffect::None, C);
	}

	void Frame(FVector2f P, FVector2f Sz, const FLinearColor& C, float Radius, float Thick) const
	{
		if (Sz.X <= 0.f || Sz.Y <= 0.f || C.A <= 0.02f) return;
		FSlateDrawElement::MakeBox(Out, Layer, Geo(P, Sz), FrameBrush(Radius * S, Thick * S, C), ESlateDrawEffect::None, FLinearColor::Transparent);
	}

	/** "Black" is the display face (Bebas Neue), "Mono" the screenplay face (Courier Prime); anything else is the engine's Roboto. */
	FSlateFontInfo Font(float Px, const TCHAR* Face = TEXT("Bold")) const
	{
		const float Size = FMath::Max(5.f, Px * S * 0.75f);
		if (FCString::Stricmp(Face, TEXT("Black")) == 0)
		{
			if (const TSharedPtr<const FCompositeFont> Display = FileFont(TEXT("BebasNeue-Regular.ttf"))) return FSlateFontInfo(Display, Size * 1.16f);
		}
		else if (FCString::Stricmp(Face, TEXT("Mono")) == 0)
		{
			if (const TSharedPtr<const FCompositeFont> Script = FileFont(TEXT("CourierPrime-Bold.ttf"))) return FSlateFontInfo(Script, Size * 1.04f);
		}
		return FCoreStyle::GetDefaultFontStyle(Face, Size);
	}

	FVector2f Measure(const FString& T, const FSlateFontInfo& F) const
	{
		const TSharedRef<FSlateFontMeasure> M = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		return FVector2f(M->Measure(T, F)) / S;
	}

	/** Align is the fraction of the text size that sits left of / above P (0 = left/top, 0.5 = centred, 1 = right/bottom). */
	void Text(const FString& T, FVector2f P, const FSlateFontInfo& F, const FLinearColor& C, float AlignX = 0.f, float AlignY = 0.f) const
	{
		if (T.IsEmpty()) return;
		const FVector2f Sz = Measure(T, F);
		FSlateDrawElement::MakeText(Out, Layer, Geo(P - FVector2f(Sz.X * AlignX, Sz.Y * AlignY), Sz), T, F, ESlateDrawEffect::None, C);
	}

	void TextRot(const FString& T, FVector2f Center, const FSlateFontInfo& F, const FLinearColor& C, float Degrees) const
	{
		const FVector2f Sz = Measure(T, F);
		const FPaintGeometry PG = G.ToPaintGeometry(Sz * S, FSlateLayoutTransform((Center - Sz * 0.5f) * S),
			FSlateRenderTransform(FQuat2f(FMath::DegreesToRadians(Degrees))), FVector2f(0.5f, 0.5f));
		FSlateDrawElement::MakeText(Out, Layer, PG, T, F, ESlateDrawEffect::None, C);
	}

	void FrameRot(FVector2f Center, FVector2f Sz, const FLinearColor& C, float Radius, float Thick, float Degrees) const
	{
		const FPaintGeometry PG = G.ToPaintGeometry(Sz * S, FSlateLayoutTransform((Center - Sz * 0.5f) * S),
			FSlateRenderTransform(FQuat2f(FMath::DegreesToRadians(Degrees))), FVector2f(0.5f, 0.5f));
		if (C.A <= 0.02f) return;
		FSlateDrawElement::MakeBox(Out, Layer, PG, FrameBrush(Radius * S, Thick * S, C), ESlateDrawEffect::None, FLinearColor::Transparent);
	}

	TArray<FString> Wrap(const FString& T, const FSlateFontInfo& F, float MaxW) const
	{
		TArray<FString> Lines, Words;
		T.ParseIntoArrayWS(Words);
		FString Cur;
		for (const FString& W : Words)
		{
			const FString Try = Cur.IsEmpty() ? W : Cur + TEXT(" ") + W;
			if (!Cur.IsEmpty() && Measure(Try, F).X > MaxW) { Lines.Add(Cur); Cur = W; }
			else Cur = Try;
		}
		if (!Cur.IsEmpty()) Lines.Add(Cur);
		return Lines;
	}

	void Line(FVector2f A, FVector2f B, const FLinearColor& C, float Thick) const
	{
		TArray<FVector2f> Pts;
		Pts.Add(A * S);
		Pts.Add(B * S);
		FSlateDrawElement::MakeLines(Out, Layer, G.ToPaintGeometry(), MoveTemp(Pts), ESlateDrawEffect::None, C, true, FMath::Max(1.f, Thick * S));
	}

	/** Filled polygon as a triangle fan around Pts[0] (so convex shapes, or star shapes with the centre first). */
	void Fan(TArrayView<const FVector2f> Pts, const FLinearColor& C) const
	{
		if (Pts.Num() < 3) return;
		const FSlateResourceHandle Handle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*WhiteBrush());
		const FSlateShaderResourceProxy* Proxy = Handle.GetResourceProxy();
		const FVector2f UV = Proxy ? FVector2f(Proxy->StartUV) + FVector2f(Proxy->SizeUV) * 0.5f : FVector2f(0.5f, 0.5f);
		const FSlateRenderTransform& RT = G.GetAccumulatedRenderTransform();
		const FColor Col = C.ToFColor(false);
		TArray<FSlateVertex> Verts;
		TArray<SlateIndex> Idx;
		Verts.Reserve(Pts.Num());
		for (const FVector2f& P : Pts) Verts.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(RT, P * S, UV, Col));
		for (int32 i = 1; i + 1 < Pts.Num(); ++i) { Idx.Add(0); Idx.Add((SlateIndex)i); Idx.Add((SlateIndex)(i + 1)); }
		FSlateDrawElement::MakeCustomVerts(Out, Layer, Handle, Verts, Idx, nullptr, 0, 0);
	}

	void Arrow(FVector2f Ctr, float Size, EMJDir Dir, const FLinearColor& C) const
	{
		const FIntPoint V = MJDirVec(Dir);
		const FVector2f F((float)V.X, (float)V.Y), R(-F.Y, F.X);
		const float H = Size * 0.5f;
		const FVector2f Head[3] = { Ctr + F * H, Ctr + R * H * 0.85f, Ctr - R * H * 0.85f };
		Fan(Head, C);
		const FVector2f Shaft[4] = { Ctr + R * H * 0.32f, Ctr - R * H * 0.32f, Ctr - R * H * 0.32f - F * H, Ctr + R * H * 0.32f - F * H };
		Fan(Shaft, C);
	}

	void Star(FVector2f Ctr, float R, const FLinearColor& C) const
	{
		FVector2f Pts[12];
		Pts[0] = Ctr;
		for (int32 i = 0; i <= 10; ++i)
		{
			const float Ang = -HALF_PI + i * PI / 5.f;
			const float Rad = (i & 1) ? R * 0.42f : R;
			Pts[i + 1] = Ctr + FVector2f(FMath::Cos(Ang), FMath::Sin(Ang)) * Rad;
		}
		Fan(Pts, C);
	}

	void Tri(FVector2f A, FVector2f B, FVector2f C3, const FLinearColor& C) const
	{
		const FVector2f Pts[3] = { A, B, C3 };
		Fan(Pts, C);
	}
};

void SMJHud::Construct(const FArguments& InArgs)
{
	Director = InArgs._Director;
	SetCanTick(false);
}

// ---------------------------------------------------------------------------------------------------------------------
// layout + hit testing

SMJHud::FLayout SMJHud::ComputeLayout(const FGeometry& Geo) const
{
	FLayout L;
	const FVector2f Sz = FVector2f(Geo.GetLocalSize());
	L.S = FMath::Max(0.05f, Sz.Y / 1080.f);
	L.W = Sz.X / L.S;
	const AMJDirector* D = Director.Get();
	if (!D) return L;

	L.NumTracks = D->Edit.Tracks.Num();
	L.RowH = L.NumTracks > 4 ? 62.f : 78.f;
	const float Toolbar = 66.f, Ruler = 34.f, Pad = 18.f;
	L.PanelH = Toolbar + Ruler + L.NumTracks * L.RowH + Pad;
	L.PanelY = 1080.f - L.PanelH;
	L.ToolbarY = L.PanelY;
	L.RulerY = L.PanelY + Toolbar;
	L.RowY0 = L.RulerY + Ruler;

	const bool bBin = (D->Level.Tools & MJTool_Insert) || D->Level.Bin.Num() > 0;
	L.BinW = bBin ? 250.f : 0.f;
	L.BinX = L.W - 24.f - L.BinW;
	L.X0 = 240.f;
	L.X1 = L.W - 24.f - (bBin ? L.BinW + 16.f : 0.f);

	int32 Beats = D->Sim.TotalBeats;
	int32 BinBeats = 0;
	for (const FMJBinClip& B : D->Level.Bin) BinBeats += FMath::Max(1, B.Clip.Beats);
	for (const FMJActorDef& A : D->Level.Actors) Beats = FMath::Max(Beats, MJ::TrackBeats(A.Track) + BinBeats);
	L.Beats = Beats + 1;
	L.BeatW = FMath::Clamp((L.X1 - L.X0) / (float)FMath::Max(L.Beats, 10), 34.f, 92.f);
	return L;
}

FVector2f SMJHud::ToVirtual(const FGeometry& Geo, const FPointerEvent& E) const
{
	const FVector2f Local = FVector2f(Geo.AbsoluteToLocal(E.GetScreenSpacePosition()));
	const float S = FMath::Max(0.05f, (float)Geo.GetLocalSize().Y / 1080.f);
	return Local / S;
}

bool SMJHud::ClipRect(const FLayout& L, int32 Track, int32 Index, FVector2f& OutPos, FVector2f& OutSize) const
{
	const AMJDirector* D = Director.Get();
	if (!D || !D->Edit.Tracks.IsValidIndex(Track) || !D->Edit.Tracks[Track].IsValidIndex(Index)) return false;
	int32 Start = 0;
	for (int32 i = 0; i < Index; ++i) Start += FMath::Max(1, D->Edit.Tracks[Track][i].Beats);
	const int32 Beats = FMath::Max(1, D->Edit.Tracks[Track][Index].Beats);
	OutPos = FVector2f(L.X0 + Start * L.BeatW + 2.f, L.RowY0 + Track * L.RowH + 8.f);
	OutSize = FVector2f(Beats * L.BeatW - 4.f, L.RowH - 16.f);
	return true;
}

bool SMJHud::HitClip(const FLayout& L, FVector2f P, int32& OutTrack, int32& OutIndex) const
{
	const AMJDirector* D = Director.Get();
	if (!D) return false;
	for (int32 t = 0; t < D->Edit.Tracks.Num(); ++t)
	{
		for (int32 i = 0; i < D->Edit.Tracks[t].Num(); ++i)
		{
			FVector2f Pos, Size;
			if (ClipRect(L, t, i, Pos, Size) && P.X >= Pos.X && P.Y >= Pos.Y && P.X <= Pos.X + Size.X && P.Y <= Pos.Y + Size.Y) { OutTrack = t; OutIndex = i; return true; }
		}
	}
	return false;
}

bool SMJHud::BinRect(const FLayout& L, int32 BinIndex, FVector2f& OutPos, FVector2f& OutSize) const
{
	if (L.BinW <= 0.f) return false;
	OutPos = FVector2f(L.BinX + 12.f, L.PanelY + 50.f + BinIndex * 50.f);
	OutSize = FVector2f(L.BinW - 24.f, 42.f);
	return true;
}

int32 SMJHud::HitBin(const FLayout& L, FVector2f P) const
{
	const AMJDirector* D = Director.Get();
	if (!D) return INDEX_NONE;
	for (int32 i = 0; i < D->Edit.Bin.Num(); ++i)
	{
		FVector2f Pos, Size;
		if (BinRect(L, i, Pos, Size) && P.X >= Pos.X && P.Y >= Pos.Y && P.X <= Pos.X + Size.X && P.Y <= Pos.Y + Size.Y) return i;
	}
	return INDEX_NONE;
}

int32 SMJHud::TrackAt(const FLayout& L, float Y, float Slack) const
{
	if (L.NumTracks <= 0 || Y < L.RowY0 - Slack || Y > L.RowY0 + L.NumTracks * L.RowH + Slack) return INDEX_NONE;
	return FMath::Clamp(FMath::FloorToInt((Y - L.RowY0) / L.RowH), 0, L.NumTracks - 1);
}

int32 SMJHud::DropIndex(const FLayout& L, int32 Track, float X, int32 SkipUid, float* OutCaretX) const
{
	const AMJDirector* D = Director.Get();
	int32 Index = 0, Beat = 0, CaretBeat = 0;
	if (D && D->Edit.Tracks.IsValidIndex(Track))
	{
		for (const FMJClip& C : D->Edit.Tracks[Track])
		{
			if (C.Uid == SkipUid) continue;
			const int32 B = FMath::Max(1, C.Beats);
			const float Center = L.X0 + (Beat + B * 0.5f) * L.BeatW;
			if (X > Center) { ++Index; CaretBeat = Beat + B; }
			Beat += B;
		}
	}
	if (OutCaretX) *OutCaretX = L.X0 + CaretBeat * L.BeatW;
	return Index;
}

FName SMJHud::HotAt(FVector2f P) const
{
	for (int32 i = Hots.Num() - 1; i >= 0; --i)
	{
		const FHot& H = Hots[i];
		if (H.bEnabled && P.X >= H.Pos.X && P.Y >= H.Pos.Y && P.X <= H.Pos.X + H.Size.X && P.Y <= H.Pos.Y + H.Size.Y) return H.Id;
	}
	return NAME_None;
}

// ---------------------------------------------------------------------------------------------------------------------
// actions

void SMJHud::CutSelection()
{
	AMJDirector* D = Director.Get();
	int32 T, I;
	if (D && D->SelUid >= 0 && D->FindClip(D->SelUid, T, I)) D->DoCut(T, I);
}

void SMJHud::Click(FName Id)
{
	AMJDirector* D = Director.Get();
	if (!D) return;
	static const FName Roll("roll"), Undo("undo"), Redo("redo"), Reset("reset"), Cut("cut"), Next("next"), Keep("keep");
	static const FName Play("play"), Scenes("scenes"), Quit("quit"), Resume("resume"), Restart("restart"), Title("title"), Hint("hint"), MenuBtn("menu"), Full("full"), Fix("fix");
	static const FName MusicDn("music-"), MusicUp("music+"), SfxDn("sfx-"), SfxUp("sfx+");
	const FString Text = Id.ToString();
	D->Sfx(TEXT("click"));
	if (Id == Roll) D->Roll();
	else if (Id == Undo) D->Undo();
	else if (Id == Redo) D->Redo();
	else if (Id == Reset) D->ResetEdits();
	else if (Id == Cut) CutSelection();
	else if (Id == Next) D->NextLevel();
	else if (Id == Keep || Id == Fix) D->BackToEdit();
	else if (Id == Hint) D->Hint();
	else if (Id == Play) D->StartGame();
	else if (Id == Scenes) D->OpenMenu(EMJMenu::Scenes);
	else if (Id == MenuBtn) D->OpenMenu(EMJMenu::Pause);
	else if (Id == Resume) D->CloseMenu();
	else if (Id == Restart) { D->CloseMenu(); D->PlayLevel(D->LevelIndex); }
	else if (Id == Title) D->GoTitle();
	else if (Id == Quit) D->QuitGame();
	else if (Id == Full) D->ToggleFullscreen();
	else if (Id == MusicDn) D->AdjustVolume(true, -0.1f);
	else if (Id == MusicUp) D->AdjustVolume(true, 0.1f);
	else if (Id == SfxDn) D->AdjustVolume(false, -0.1f);
	else if (Id == SfxUp) D->AdjustVolume(false, 0.1f);
	else if (Text.StartsWith(TEXT("scene_"))) D->PlayLevel(FCString::Atoi(*Text.Mid(6)));
}

FReply SMJHud::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	AMJDirector* D = Director.Get();
	if (!D || MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton) return FReply::Handled();
	const FLayout L = ComputeLayout(MyGeometry);
	const FVector2f P = ToVirtual(MyGeometry, MouseEvent);
	Mouse = P;
	Drag = EDrag::None;
	bDragMoved = false;
	DragStart = P;
	PressedHot = HotAt(P);

	FReply Reply = FReply::Handled().SetUserFocus(AsShared(), EFocusCause::Mouse).CaptureMouse(AsShared());
	if (!PressedHot.IsNone() || IsModal()) return Reply;

	if (D->Phase == EMJPhase::Failed) { D->BackToEdit(); return Reply; }
	if (D->Phase != EMJPhase::Edit) return Reply;

	int32 T, I;
	if (HitClip(L, P, T, I))
	{
		FVector2f Pos, Size;
		ClipRect(L, T, I, Pos, Size);
		D->SelUid = D->Edit.Tracks[T][I].Uid;
		if (MJ::CanEditClip(D->Level, D->Edit, T, I))
		{
			Drag = EDrag::Clip; DragTrack = T; DragIndex = I; DragUid = D->SelUid; DragGrab = P - Pos;
		}
		D->Sfx(TEXT("select"));
		return Reply;
	}
	const int32 B = HitBin(L, P);
	if (B != INDEX_NONE)
	{
		FVector2f Pos, Size;
		BinRect(L, B, Pos, Size);
		Drag = EDrag::Bin; DragIndex = B; DragTrack = D->Edit.Bin[B].Actor; DragUid = D->Edit.Bin[B].Clip.Uid; DragGrab = P - Pos;
		D->SelUid = -1;
		D->Sfx(TEXT("select"));
		return Reply;
	}
	D->SelUid = -1;
	if (P.Y >= L.RulerY && P.X >= L.X0 - 12.f && P.X <= L.X1 + 12.f)
	{
		Drag = EDrag::Scrub;
		D->Scrub((P.X - L.X0) / L.BeatW);
	}
	return Reply;
}

FReply SMJHud::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	AMJDirector* D = Director.Get();
	const FVector2f P = ToVirtual(MyGeometry, MouseEvent);
	Mouse = P;
	if (!D) return FReply::Unhandled();
	if (Drag == EDrag::Scrub)
	{
		const FLayout L = ComputeLayout(MyGeometry);
		D->Scrub((P.X - L.X0) / L.BeatW);
	}
	else if (Drag != EDrag::None && !bDragMoved && FVector2f::Distance(P, DragStart) > 7.f)
	{
		bDragMoved = true;
		D->Sfx(TEXT("pickup"));
	}
	return FReply::Handled();
}

FReply SMJHud::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	AMJDirector* D = Director.Get();
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton) return FReply::Handled();
	const FVector2f P = D ? ToVirtual(MyGeometry, MouseEvent) : FVector2f::ZeroVector;
	Mouse = P;
	if (D)
	{
		const FLayout L = ComputeLayout(MyGeometry);
		if (!PressedHot.IsNone())
		{
			if (HotAt(P) == PressedHot) Click(PressedHot);
		}
		else if (Drag == EDrag::Clip && bDragMoved)
		{
			int32 T, I;
			if (D->FindClip(DragUid, T, I))
			{
				FVector2f Pos, Size;
				ClipRect(L, T, I, Pos, Size);
				const int32 Over = TrackAt(L, P.Y, 26.f);
				if (Over == T) D->DoMove(T, I, DropIndex(L, T, P.X - DragGrab.X + Size.X * 0.5f, DragUid));
				else if (Over == INDEX_NONE) D->DoCut(T, I);
			}
		}
		else if (Drag == EDrag::Bin && bDragMoved)
		{
			const int32 Over = TrackAt(L, P.Y, 20.f);
			int32 BinIndex = INDEX_NONE;
			for (int32 i = 0; i < D->Edit.Bin.Num(); ++i) { if (D->Edit.Bin[i].Clip.Uid == DragUid) BinIndex = i; }
			if (Over != INDEX_NONE && BinIndex != INDEX_NONE && P.X < L.X1 + 10.f) D->DoInsert(BinIndex, Over, DropIndex(L, Over, P.X, -1));
		}
	}
	PressedHot = NAME_None;
	Drag = EDrag::None;
	bDragMoved = false;
	return FReply::Handled().ReleaseMouseCapture();
}

FReply SMJHud::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	AMJDirector* D = Director.Get();
	if (D && D->Phase == EMJPhase::Edit && !IsModal() && HotAt(ToVirtual(MyGeometry, MouseEvent)).IsNone() && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		const FLayout L = ComputeLayout(MyGeometry);
		int32 T, I;
		if (HitClip(L, ToVirtual(MyGeometry, MouseEvent), T, I)) { D->DoCut(T, I); return FReply::Handled(); }
	}
	return OnMouseButtonDown(MyGeometry, MouseEvent);
}

void SMJHud::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	if (Drag == EDrag::None) Mouse = FVector2f(-1000.f, -1000.f);
}

FReply SMJHud::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
	AMJDirector* D = Director.Get();
	if (!D) return FReply::Unhandled();
	const FKey K = KeyEvent.GetKey();
	const bool bGo = K == EKeys::SpaceBar || K == EKeys::Enter;

	if (K == EKeys::F11 && !KeyEvent.IsRepeat()) { D->ToggleFullscreen(); return FReply::Handled(); }
	if (K == EKeys::F9 && !KeyEvent.IsRepeat()) { D->ToggleAudioCapture(); return FReply::Handled(); }
	if (K == EKeys::F10 && !KeyEvent.IsRepeat()) { FScreenshotRequest::RequestScreenshot(true); return FReply::Handled(); } // engine-side screenshot with UI: works with the monitor off
	if (D->Menu != EMJMenu::None)
	{
		if (K == EKeys::Escape && !KeyEvent.IsRepeat()) { D->CloseMenu(); return FReply::Handled(); }
		return FReply::Unhandled();
	}
	if (K == EKeys::F) { D->bFast = true; return FReply::Handled(); }
	if (K == EKeys::PageDown && !KeyEvent.IsRepeat()) { D->LoadLevel(FMath::Min(D->LevelIndex + 1, D->NumLevels() - 1)); return FReply::Handled(); }
	if (K == EKeys::PageUp && !KeyEvent.IsRepeat()) { D->LoadLevel(D->LevelIndex - 1); return FReply::Handled(); }

	switch (D->Phase)
	{
	case EMJPhase::Title:
		if (bGo && !KeyEvent.IsRepeat()) { D->StartGame(); return FReply::Handled(); }
		break;
	case EMJPhase::Credits:
		if ((bGo || K == EKeys::Escape) && !KeyEvent.IsRepeat()) { D->GoTitle(); return FReply::Handled(); }
		break;
	case EMJPhase::Edit:
		if (bGo && !KeyEvent.IsRepeat()) D->Roll();
		else if (K == EKeys::X || K == EKeys::Delete || K == EKeys::BackSpace) CutSelection();
		else if (K == EKeys::Z) D->Undo();
		else if (K == EKeys::Y) D->Redo();
		else if (K == EKeys::R && !KeyEvent.IsRepeat()) D->ResetEdits();
		else if (K == EKeys::Left) D->StepBeat(-1);
		else if (K == EKeys::Right) D->StepBeat(1);
		else if (K == EKeys::H && !KeyEvent.IsRepeat()) D->Hint();
		else if (K == EKeys::Escape && !KeyEvent.IsRepeat()) { if (D->SelUid >= 0) D->SelUid = -1; else D->OpenMenu(EMJMenu::Pause); }
		else return FReply::Unhandled();
		return FReply::Handled();
	case EMJPhase::Screening:
		if ((bGo || K == EKeys::Escape) && !KeyEvent.IsRepeat()) { D->BackToEdit(); return FReply::Handled(); }
		break;
	case EMJPhase::Failed:
		if ((bGo || K == EKeys::Escape || K == EKeys::R) && !KeyEvent.IsRepeat()) { D->BackToEdit(); return FReply::Handled(); }
		break;
	case EMJPhase::Printed:
		if (bGo && !KeyEvent.IsRepeat()) { D->NextLevel(); return FReply::Handled(); }
		if (K == EKeys::Escape) { D->BackToEdit(); return FReply::Handled(); }
		break;
	}
	return FReply::Unhandled();
}

FReply SMJHud::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
	AMJDirector* D = Director.Get();
	if (D && KeyEvent.GetKey() == EKeys::F) { D->bFast = false; return FReply::Handled(); }
	return FReply::Unhandled();
}

FCursorReply SMJHud::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	const AMJDirector* D = Director.Get();
	if (Drag == EDrag::Clip || Drag == EDrag::Bin) return FCursorReply::Cursor(EMouseCursor::GrabHandClosed);
	if (!HotAt(Mouse).IsNone()) return FCursorReply::Cursor(EMouseCursor::Hand);
	if (D && D->Phase == EMJPhase::Edit)
	{
		const FLayout L = ComputeLayout(MyGeometry);
		int32 T, I;
		if (HitClip(L, Mouse, T, I)) return FCursorReply::Cursor(MJ::CanEditClip(D->Level, D->Edit, T, I) ? EMouseCursor::GrabHand : EMouseCursor::SlashedCircle);
		if (HitBin(L, Mouse) != INDEX_NONE) return FCursorReply::Cursor(EMouseCursor::GrabHand);
	}
	return FCursorReply::Cursor(EMouseCursor::Default);
}

// ---------------------------------------------------------------------------------------------------------------------
// painting

bool SMJHud::Button(FMJPaint& P, FName Id, FVector2f Pos, FVector2f Size, const FString& Label, const FLinearColor& Color, bool bEnabled, const FString& Key) const
{
	FHot H; H.Id = Id; H.Pos = Pos; H.Size = Size; H.bEnabled = bEnabled;
	Hots.Add(H);
	const AMJDirector* Dir = Director.Get();
	const bool bCovered = Dir && Dir->Menu != EMJMenu::None && !bInMenuPaint; // a menu is drawn over this button
	const bool bHover = bEnabled && !bCovered && Mouse.X >= Pos.X && Mouse.Y >= Pos.Y && Mouse.X <= Pos.X + Size.X && Mouse.Y <= Pos.Y + Size.Y;
	const bool bDown = bHover && PressedHot == Id;
	const FLinearColor Fill = !bEnabled ? Hex(0x23262E) : (bDown ? Shade(Color, 0.75f) : (bHover ? Shade(Color, 1.15f) : Color));
	const FVector2f Off(0.f, bDown ? 2.f : 0.f);
	P.Box(Pos + FVector2f(0.f, 4.f), Size, Hex(0x000000, 0.45f), 9.f);
	P.Box(Pos + Off, Size, Fill, 9.f);
	const FLinearColor TextCol = !bEnabled ? Hex(0x5A6070) : (Color.GetLuminance() > 0.35f ? ColInk : ColText);
	if (Key.IsEmpty())
	{
		P.Text(Label, Pos + Off + Size * 0.5f, P.Font(19.f), TextCol, 0.5f, 0.5f);
	}
	else
	{
		P.Text(Label, Pos + Off + FVector2f(Size.X * 0.5f, Size.Y * 0.40f), P.Font(18.f), TextCol, 0.5f, 0.5f);
		P.Text(Key, Pos + Off + FVector2f(Size.X * 0.5f, Size.Y * 0.80f), P.Font(10.5f, TEXT("Regular")), Fade(TextCol, 0.7f), 0.5f, 0.5f);
	}
	return bHover;
}

void SMJHud::PaintClip(FMJPaint& P, const FMJClip& C, int32 OwnerTrack, FVector2f Pos, FVector2f Size, bool bLocked, bool bSelected, bool bHover, float Alpha, bool bPlaying) const
{
	FLinearColor Col = ActColor(C);
	if (bLocked) Col = FMath::Lerp(Col, Hex(0x3A3D46), 0.62f);
	if (bHover && !bLocked) Col = Shade(Col, 1.18f);

	P.Box(Pos + FVector2f(0.f, 3.f), Size, Hex(0x000000, 0.5f * Alpha), 8.f);
	P.Box(Pos, Size, Fade(Col, Alpha), 8.f);
	P.Box(Pos + FVector2f(3.f, 3.f), FVector2f(Size.X - 6.f, Size.Y * 0.38f), Hex(0xFFFFFF, 0.10f * Alpha), 6.f);
	if (C.Orig < 0) P.Frame(Pos, Size, Fade(ColGreen, Alpha), 8.f, 2.f); // restored deleted scene
	if (bSelected) P.Frame(Pos - FVector2f(3.f, 3.f), Size + FVector2f(6.f, 6.f), Fade(ColText, Alpha), 10.f, 3.f);
	else if (bPlaying) P.Frame(Pos - FVector2f(2.f, 2.f), Size + FVector2f(4.f, 4.f), Fade(ColAmber, Alpha), 10.f, 3.f);

	const int32 Beats = FMath::Max(1, C.Beats);
	const float BW = Size.X / Beats;
	for (int32 b = 1; b < Beats; ++b) P.Box(Pos + FVector2f(b * BW - 1.f, 6.f), FVector2f(2.f, Size.Y - 12.f), Hex(0x000000, 0.22f * Alpha));

	const FLinearColor Ink = Fade(bLocked ? Hex(0xC9CDD6) : Hex(0xFFFFFF), Alpha);
	const FString Label = MJ::ClipLabel(C);
	if (C.Act == EMJAct::Move || C.Act == EMJAct::Look)
	{
		const bool bLook = C.Act == EMJAct::Look;
		const float ArrowSize = FMath::Min(Size.Y * (Label.IsEmpty() ? 0.50f : 0.40f), BW * 0.6f);
		for (int32 b = 0; b < Beats; ++b)
		{
			const FVector2f Ctr = Pos + FVector2f((b + 0.5f) * BW, Label.IsEmpty() ? Size.Y * 0.5f : Size.Y * 0.38f);
			if (bLook) P.Frame(Ctr - FVector2f(ArrowSize * 0.75f), FVector2f(ArrowSize * 1.5f), Fade(Ink, 0.55f), ArrowSize * 0.75f, 2.f);
			P.Arrow(Ctr, bLook ? ArrowSize * 0.8f : ArrowSize, C.Dir, Ink);
		}
		if (!Label.IsEmpty()) P.Text(Label, Pos + FVector2f(Size.X * 0.5f, Size.Y * 0.78f), P.Font(12.f), Ink, 0.5f, 0.5f);
	}
	else
	{
		float Px = 17.f;
		FSlateFontInfo F = P.Font(Px);
		const float MaxW = Size.X - 10.f;
		TArray<FString> Lines = P.Wrap(Label, F, MaxW);
		while (Px > 10.f && (Lines.Num() > 2 || (Lines.Num() > 0 && P.Measure(Lines[0], F).X > MaxW)))
		{
			Px -= 1.5f;
			F = P.Font(Px);
			Lines = P.Wrap(Label, F, MaxW);
		}
		const float LineH = Px * 1.15f;
		float Y = Pos.Y + Size.Y * 0.5f - (Lines.Num() - 1) * LineH * 0.5f;
		for (const FString& Ln : Lines) { P.Text(Ln, FVector2f(Pos.X + Size.X * 0.5f, Y), F, Ink, 0.5f, 0.5f); Y += LineH; }
	}
	if (bLocked)
	{
		// padlock
		const FVector2f K = Pos + FVector2f(Size.X - 15.f, 6.f);
		P.Frame(K + FVector2f(2.f, 0.f), FVector2f(8.f, 9.f), Fade(Hex(0xE8E8E8), Alpha), 4.f, 1.6f);
		P.Box(K + FVector2f(0.f, 5.f), FVector2f(12.f, 9.f), Fade(Hex(0xE8E8E8), Alpha), 2.f);
	}
}

void SMJHud::PaintTopBar(FMJPaint& P, const FLayout& L) const
{
	const AMJDirector* D = Director.Get();
	const FMJLevelDef& Lv = D->Level;

	if (D->Phase == EMJPhase::Screening)
	{
		// letterbox + recording slate
		const float K = FMath::Clamp(D->PhaseTime / 0.25f, 0.f, 1.f);
		P.Box(FVector2f(0.f, 0.f), FVector2f(L.W, 74.f * K), Hex(0x000000));
		P.Box(FVector2f(0.f, L.PanelY - 26.f * K), FVector2f(L.W, 26.f * K), Hex(0x000000));
		if (FMath::Fmod(D->PhaseTime, 1.0f) < 0.6f) P.Box(FVector2f(34.f, 28.f), FVector2f(18.f, 18.f), ColRed, 9.f);
		P.Text(D->bIntro ? TEXT("THE THEATRICAL CUT") : TEXT("SCREENING YOUR CUT"), FVector2f(64.f, 37.f), P.Font(18.f), D->bIntro ? ColAmber : ColText, 0.f, 0.5f);
		const float Secs = D->PlayTime / FMath::Max(0.1f, D->BeatsPerSecond);
		const int32 Frames = FMath::FloorToInt(Secs * 24.f);
		P.Text(FString::Printf(TEXT("%02d:%02d:%02d"), Frames / 1440, (Frames / 24) % 60, Frames % 24), FVector2f(L.W - 34.f, 37.f), P.Font(20.f, TEXT("Mono")), ColText, 1.f, 0.5f);
		P.Text(FString::Printf(TEXT("%s  -  %s"), *Lv.Title.ToUpper(), *Lv.Slug), FVector2f(L.W * 0.5f, 37.f), P.Font(16.f, TEXT("Regular")), ColDim, 0.5f, 0.5f);
		return;
	}

	P.Box(FVector2f(0.f, 0.f), FVector2f(L.W, 112.f), Hex(0x0B0C10, 0.86f));
	P.Box(FVector2f(0.f, 112.f), FVector2f(L.W, 2.f), Hex(0xFFB020, 0.5f));

	P.Text(FString::Printf(TEXT("REEL %d   SCENE %d"), Lv.Reel, D->LevelIndex + 1), FVector2f(28.f, 14.f), P.Font(14.f), ColAmber);
	P.Text(Lv.Title.ToUpper(), FVector2f(28.f, 30.f), P.Font(42.f, TEXT("Black")), ColText);
	P.Text(Lv.Slug, FVector2f(28.f, 80.f), P.Font(14.f, TEXT("Mono")), ColDim);

	// the script page
	const float CardX = 520.f, CardW = FMath::Max(300.f, L.W - CardX - 400.f);
	P.Box(FVector2f(CardX + 4.f, 16.f), FVector2f(CardW, 84.f), Hex(0x000000, 0.4f), 4.f);
	P.Box(FVector2f(CardX, 12.f), FVector2f(CardW, 84.f), ColPaper, 4.f);
	P.Text(TEXT("THE SCRIPT SAYS"), FVector2f(CardX + 16.f, 19.f), P.Font(11.f), Hex(0x8A6A1E));
	const FSlateFontInfo ScriptFont = P.Font(19.f, TEXT("Mono"));
	const TArray<FString> Lines = P.Wrap(Lv.Script, ScriptFont, CardW - 32.f);
	float Y = Lines.Num() > 1 ? 38.f : 50.f;
	for (int32 i = 0; i < Lines.Num() && i < 2; ++i) { P.Text(Lines[i], FVector2f(CardX + 16.f, Y), ScriptFont, ColInk); Y += 25.f; }

	// edit budget + bonus
	const float RX = L.W - 28.f;
	P.Text(TEXT("EDITS"), FVector2f(RX - 372.f + 372.f - 0.f, 16.f), P.Font(13.f), ColDim, 1.f, 0.f);
	const int32 Max = Lv.MaxEdits, Used = D->Cost();
	for (int32 i = 0; i < Max; ++i)
	{
		const FVector2f C(RX - 14.f - (Max - 1 - i) * 34.f, 54.f);
		const bool bUsed = i < Used;
		P.Box(C - FVector2f(13.f, 13.f), FVector2f(26.f, 26.f), bUsed ? Hex(0x2A2D36) : ColAmber, 13.f);
		if (bUsed) P.Line(C - FVector2f(7.f, 7.f), C + FVector2f(7.f, 7.f), ColRed, 3.f), P.Line(C + FVector2f(-7.f, 7.f), C + FVector2f(7.f, -7.f), ColRed, 3.f);
	}
	P.Text(FString::Printf(TEXT("%d of %d used   par %d"), Used, Max, Lv.Par), FVector2f(RX, 72.f), P.Font(13.f, TEXT("Regular")), ColDim, 1.f, 0.f);
	if (Lv.Bonus != EMJBonus::None)
	{
		const FSlateFontInfo BF = P.Font(13.f, TEXT("Regular"));
		P.Text(Lv.BonusText, FVector2f(RX, 101.f), BF, ColText, 1.f, 0.5f);
		P.Star(FVector2f(RX - P.Measure(Lv.BonusText, BF).X - 14.f, 100.f), 8.f, (D->StarMask(Lv.Id) & 4) ? ColAmber : Hex(0x6B7180));
	}
}

void SMJHud::PaintPanel(FMJPaint& P, const FLayout& L) const
{
	const AMJDirector* D = Director.Get();
	const FMJLevelDef& Lv = D->Level;
	const bool bEdit = D->Phase == EMJPhase::Edit;

	P.Box(FVector2f(0.f, L.PanelY), FVector2f(L.W, L.PanelH), ColPanel);
	P.Box(FVector2f(0.f, L.PanelY), FVector2f(L.W, 2.f), Hex(0xFFB020, 0.55f));

	// ---- toolbar
	const float BY = L.ToolbarY + 11.f, BH = 46.f;
	float BX = 24.f;
	const bool bScreening = D->Phase == EMJPhase::Screening;
	Button(P, TEXT("roll"), FVector2f(BX, BY), FVector2f(190.f, BH), bScreening ? TEXT("STOP") : TEXT("ROLL FILM"), bScreening ? ColRed : ColAmber, D->Phase == EMJPhase::Edit || bScreening, TEXT("SPACE"));
	BX += 216.f;
	int32 SelT = -1, SelI = -1;
	const bool bSel = bEdit && D->SelUid >= 0 && D->FindClip(D->SelUid, SelT, SelI) && MJ::CanEditClip(Lv, D->Edit, SelT, SelI);
	Button(P, TEXT("cut"), FVector2f(BX, BY), FVector2f(120.f, BH), TEXT("CUT"), Hex(0xD4513B), bSel, TEXT("X"));
	BX += 132.f;
	Button(P, TEXT("undo"), FVector2f(BX, BY), FVector2f(104.f, BH), TEXT("UNDO"), Hex(0x343946), bEdit && D->CanUndo(), TEXT("Z"));
	BX += 114.f;
	Button(P, TEXT("redo"), FVector2f(BX, BY), FVector2f(104.f, BH), TEXT("REDO"), Hex(0x343946), bEdit && D->CanRedo(), TEXT("Y"));
	BX += 114.f;
	Button(P, TEXT("reset"), FVector2f(BX, BY), FVector2f(170.f, BH), TEXT("THEATRICAL CUT"), Hex(0x343946), bEdit && D->Cost() > 0, TEXT("R  -  RESET"));
	BX += 190.f;

	const float ToolRight = L.BinW > 0.f ? L.BinX - 16.f : L.W - 24.f; // the deleted-scenes bin owns the right column
	Button(P, TEXT("menu"), FVector2f(ToolRight - 110.f, BY), FVector2f(110.f, BH), TEXT("MENU"), Hex(0x343946), D->Phase != EMJPhase::Printed, TEXT("ESC"));
	Button(P, TEXT("hint"), FVector2f(ToolRight - 110.f - 12.f - 110.f, BY), FVector2f(110.f, BH), TEXT("HINT"), Hex(0x2B5F8A), bEdit, TEXT("H"));

	if (D->ToastTime > 0.f && !D->Toast.IsEmpty())
	{
		const float A = FMath::Clamp(D->ToastTime / 0.4f, 0.f, 1.f);
		P.Text(D->Toast, FVector2f(BX + 8.f, BY + BH * 0.5f), P.Font(18.f), Fade(ColRed, A), 0.f, 0.5f);
	}
	else if (bEdit)
	{
		FString Tools = TEXT("CUT: X or drag off");
		if (Lv.Tools & MJTool_Move) Tools += TEXT("    SPLICE: drag along the strip");
		if (Lv.Tools & MJTool_Insert) Tools += TEXT("    RESTORE: drag from the bin");
		const FSlateFontInfo ToolFont = P.Font(14.f, TEXT("Regular"));
		if (BX + 8.f + P.Measure(Tools, ToolFont).X < ToolRight - 250.f) P.Text(Tools, FVector2f(BX + 8.f, BY + BH * 0.5f), ToolFont, ColDim, 0.f, 0.5f);
	}

	// ---- ruler
	const float RowsH = L.NumTracks * L.RowH;
	for (int32 b = 0; b <= L.Beats; ++b)
	{
		const float X = L.X0 + b * L.BeatW;
		if (X > L.X1 + 1.f) break;
		P.Box(FVector2f(X - 0.5f, L.RulerY + 20.f), FVector2f(1.f, 14.f + RowsH), Hex(0xFFFFFF, 0.07f));
		if (b < L.Beats && X + L.BeatW <= L.X1 + 1.f) P.Text(FString::FromInt(b + 1), FVector2f(X + L.BeatW * 0.5f, L.RulerY + 10.f), P.Font(12.f, TEXT("Regular")), ColDim, 0.5f, 0.5f);
	}

	// ---- strips
	const int32 PlayBeat = FMath::Min(FMath::FloorToInt(D->PlayTime), FMath::Max(0, D->Sim.EndBeat - 1));
	int32 HoverT = -1, HoverI = -1;
	if (bEdit && Drag == EDrag::None) HitClip(L, Mouse, HoverT, HoverI);

	for (int32 t = 0; t < L.NumTracks; ++t)
	{
		const FMJActorDef& A = Lv.Actors[t];
		const float RY = L.RowY0 + t * L.RowH;
		const FLinearColor RC = RoleColor(A.Role);

		// header
		P.Box(FVector2f(24.f, RY + 6.f), FVector2f(200.f, L.RowH - 12.f), Hex(0x1A1D25), 8.f);
		P.Box(FVector2f(24.f, RY + 6.f), FVector2f(7.f, L.RowH - 12.f), RC, 3.f);
		P.Text(A.Name, FVector2f(42.f, RY + L.RowH * 0.5f - 9.f), P.Font(20.f), ColText, 0.f, 0.5f);
		P.Text(A.bLocked ? FString(TEXT("STUDIO LOCKED")) : FString(RoleText(A.Role)), FVector2f(42.f, RY + L.RowH * 0.5f + 14.f), P.Font(11.f), A.bLocked ? ColRed : RC, 0.f, 0.5f);

		// film strip
		const FVector2f SP(L.X0 - 8.f, RY + 2.f), SS(L.X1 - L.X0 + 16.f, L.RowH - 4.f);
		P.Box(SP, SS, ColStrip, 5.f);
		for (float X = SP.X + 8.f; X < SP.X + SS.X - 12.f; X += 21.f)
		{
			P.Box(FVector2f(X, SP.Y + 2.f), FVector2f(9.f, 4.f), ColHole, 1.5f);
			P.Box(FVector2f(X, SP.Y + SS.Y - 6.f), FVector2f(9.f, 4.f), ColHole, 1.5f);
		}

		const TArray<FMJClip>& Clips = D->Edit.Tracks[t];
		const int16 PlayingClip = (D->Sim.Steps.IsValidIndex(PlayBeat) && (D->Phase != EMJPhase::Edit || D->PlayTime > 0.f)) ? D->Sim.Steps[PlayBeat].Actors[t].Clip : (int16)-1;
		for (int32 i = 0; i < Clips.Num(); ++i)
		{
			FVector2f Pos, Size;
			ClipRect(L, t, i, Pos, Size);
			if (Pos.X > L.X1) break;
			const bool bDragged = Drag == EDrag::Clip && bDragMoved && Clips[i].Uid == DragUid;
			const bool bLocked = A.bLocked || Clips[i].bLocked;
			PaintClip(P, Clips[i], t, Pos, Size, bLocked, Clips[i].Uid == D->SelUid && !bDragged, t == HoverT && i == HoverI, bDragged ? 0.25f : 1.f, i == PlayingClip);
			if (D->HintTime > 0.f && Clips[i].Uid == D->HintUid)
			{
				const float Pulse = 0.55f + 0.45f * FMath::Sin((float)FPlatformTime::Seconds() * 7.f);
				P.Frame(Pos - FVector2f(5.f, 5.f), Size + FVector2f(10.f, 10.f), Fade(Hex(0x5CC8FF), Pulse), 12.f, 4.f);
			}
		}
	}

	// ---- where the film stops
	const float EndX = L.X0 + D->Sim.EndBeat * L.BeatW;
	if (EndX < L.X1)
	{
		P.Box(FVector2f(EndX, L.RowY0), FVector2f(L.X1 + 8.f - EndX, RowsH), Hex(0x000000, 0.42f));
		const FLinearColor MC = D->Sim.IsWin() ? ColGreen : ColRed;
		P.Box(FVector2f(EndX - 1.5f, L.RulerY + 4.f), FVector2f(3.f, 30.f + RowsH), MC);
		const FString EndTag = D->Sim.IsWin() ? TEXT("PRINT") : (D->Sim.Outcome == EMJOutcome::OutOfFilm ? TEXT("FILM ENDS") : TEXT("CUT!"));
		const FSlateFontInfo TF = P.Font(12.f);
		const float TW = P.Measure(EndTag, TF).X + 14.f;
		P.Box(FVector2f(EndX + 1.5f, L.RulerY + 2.f), FVector2f(TW, 20.f), MC, 3.f);
		P.Text(EndTag, FVector2f(EndX + 1.5f + TW * 0.5f, L.RulerY + 12.f), TF, D->Sim.IsWin() ? ColInk : ColText, 0.5f, 0.5f);
	}

	// ---- playhead
	{
		const float X = L.X0 + D->PlayTime * L.BeatW;
		P.Box(FVector2f(X - 1.5f, L.RulerY + 2.f), FVector2f(3.f, 32.f + RowsH), ColText);
		P.Tri(FVector2f(X - 9.f, L.RulerY - 2.f), FVector2f(X + 9.f, L.RulerY - 2.f), FVector2f(X, L.RulerY + 12.f), ColText);
	}

	// ---- deleted scenes bin
	if (L.BinW > 0.f)
	{
		P.Box(FVector2f(L.BinX, L.PanelY + 10.f), FVector2f(L.BinW, L.PanelH - 20.f), Hex(0x191C24), 8.f);
		P.Frame(FVector2f(L.BinX, L.PanelY + 10.f), FVector2f(L.BinW, L.PanelH - 20.f), Hex(0x3DD68C, 0.5f), 8.f, 1.5f);
		P.Text(TEXT("DELETED SCENES"), FVector2f(L.BinX + 14.f, L.PanelY + 20.f), P.Font(14.f), ColGreen);
		if (D->Edit.Bin.Num() == 0) P.Text(TEXT("(empty)"), FVector2f(L.BinX + 14.f, L.PanelY + 54.f), P.Font(13.f, TEXT("Regular")), ColDim);
		const int32 HoverB = (bEdit && Drag == EDrag::None) ? HitBin(L, Mouse) : INDEX_NONE;
		for (int32 i = 0; i < D->Edit.Bin.Num(); ++i)
		{
			FVector2f Pos, Size;
			BinRect(L, i, Pos, Size);
			const bool bDragged = Drag == EDrag::Bin && bDragMoved && D->Edit.Bin[i].Clip.Uid == DragUid;
			const FVector2f CS(FMath::Min(Size.X - 96.f, 120.f), Size.Y);
			PaintClip(P, D->Edit.Bin[i].Clip, D->Edit.Bin[i].Actor, Pos, CS, false, false, i == HoverB, bDragged ? 0.25f : 1.f, false);
			if (D->HintTime > 0.f && D->Edit.Bin[i].Clip.Uid == D->HintUid)
			{
				const float Pulse = 0.55f + 0.45f * FMath::Sin((float)FPlatformTime::Seconds() * 7.f);
				P.Frame(Pos - FVector2f(5.f, 5.f), CS + FVector2f(10.f, 10.f), Fade(Hex(0x5CC8FF), Pulse), 12.f, 4.f);
			}
			const FMJActorDef& Owner = Lv.Actors[D->Edit.Bin[i].Actor];
			P.Text(TEXT("for"), FVector2f(Pos.X + CS.X + 10.f, Pos.Y + 11.f), P.Font(10.f, TEXT("Regular")), ColDim, 0.f, 0.5f);
			P.Text(Owner.Name, FVector2f(Pos.X + CS.X + 10.f, Pos.Y + 27.f), P.Font(13.f), RoleColor(Owner.Role), 0.f, 0.5f);
		}
	}

	// ---- what a clip does, on hover
	if (HoverT >= 0 && D->Edit.Tracks.IsValidIndex(HoverT) && D->Edit.Tracks[HoverT].IsValidIndex(HoverI))
	{
		static const TCHAR* DirName[] = { TEXT("north"), TEXT("east"), TEXT("south"), TEXT("west"), TEXT("") };
		const FMJClip& C = D->Edit.Tracks[HoverT][HoverI];
		const TCHAR* Dir = DirName[FMath::Clamp((int32)C.Dir, 0, 4)];
		FString Tip;
		switch (C.Act)
		{
		case EMJAct::Move: Tip = C.Beats > 1 ? FString::Printf(TEXT("Runs %d tiles %s, one per beat"), C.Beats, Dir) : FString::Printf(TEXT("Steps one tile %s"), Dir); break;
		case EMJAct::Wait: Tip = C.Beats > 1 ? FString::Printf(TEXT("Stands still for %d beats"), C.Beats) : FString(TEXT("Stands still for one beat")); break;
		case EMJAct::Look: Tip = FString::Printf(TEXT("Turns to face %s (guards see three tiles ahead)"), Dir); break;
		case EMJAct::Use: Tip = TEXT("Uses what is on this tile: pulls the lever or grabs the prize"); break;
		case EMJAct::Attack: Tip = TEXT("Knocks out whoever stands on the tile in front"); break;
		case EMJAct::Chan: Tip = (C.Param & 0x100) ? TEXT("Switches the lasers off") : TEXT("Switches the lasers back on"); break;
		}
		if (!MJ::CanEditClip(Lv, D->Edit, HoverT, HoverI)) Tip += TEXT("   -   locked by the studio");
		FVector2f Pos, Size;
		ClipRect(L, HoverT, HoverI, Pos, Size);
		const FSlateFontInfo TF = P.Font(15.f, TEXT("Regular"));
		const float TW = P.Measure(Tip, TF).X + 24.f;
		const float TX = FMath::Clamp(Pos.X + Size.X * 0.5f - TW * 0.5f, 12.f, L.W - TW - 12.f);
		++P.Layer;
		P.Box(FVector2f(TX, Pos.Y - 40.f), FVector2f(TW, 30.f), Hex(0x05060A, 0.97f), 6.f);
		P.Text(Tip, FVector2f(TX + 12.f, Pos.Y - 25.f), TF, ColText, 0.f, 0.5f);
	}

	// ---- drag feedback
	if (bEdit && bDragMoved && (Drag == EDrag::Clip || Drag == EDrag::Bin))
	{
		const FMJClip* C = nullptr;
		int32 T = -1, I = -1;
		if (Drag == EDrag::Clip) { if (D->FindClip(DragUid, T, I)) C = &D->Edit.Tracks[T][I]; }
		else { for (const FMJBinClip& B : D->Edit.Bin) { if (B.Clip.Uid == DragUid) { C = &B.Clip; T = B.Actor; } } }
		if (C)
		{
			const FVector2f Size(FMath::Max(1, C->Beats) * L.BeatW - 4.f, L.RowH - 16.f);
			const FVector2f Pos = Mouse - FVector2f(FMath::Min(DragGrab.X, Size.X - 4.f), FMath::Min(DragGrab.Y, Size.Y - 4.f));
			const int32 Over = TrackAt(L, Mouse.Y, Drag == EDrag::Clip ? 26.f : 20.f);
			const bool bCanDrop = Over == T && Mouse.X < L.X1 + 10.f;
			if (bCanDrop)
			{
				float CaretX = 0.f;
				DropIndex(L, T, Drag == EDrag::Clip ? Pos.X + Size.X * 0.5f : Mouse.X, Drag == EDrag::Clip ? DragUid : -1, &CaretX);
				const float RY = L.RowY0 + T * L.RowH;
				P.Box(FVector2f(CaretX - 2.5f, RY + 2.f), FVector2f(5.f, L.RowH - 4.f), ColAmber, 2.f);
				P.Tri(FVector2f(CaretX - 9.f, RY - 4.f), FVector2f(CaretX + 9.f, RY - 4.f), FVector2f(CaretX, RY + 8.f), ColAmber);
			}
			++P.Layer;
			PaintClip(P, *C, T, Pos, Size, false, false, false, 0.92f, false);
			if (Drag == EDrag::Clip && Over == INDEX_NONE)
			{
				P.Box(Pos + FVector2f(Size.X * 0.5f - 44.f, -30.f), FVector2f(88.f, 24.f), ColRed, 5.f);
				P.Text(TEXT("CUT"), Pos + FVector2f(Size.X * 0.5f, -18.f), P.Font(14.f), ColText, 0.5f, 0.5f);
			}
		}
	}
}

void SMJHud::PaintOverlays(FMJPaint& P, const FLayout& L) const
{
	const AMJDirector* D = Director.Get();
	const FMJLevelDef& Lv = D->Level;
	const FVector2f Mid(L.W * 0.5f, (114.f + L.PanelY) * 0.5f);

	if (D->Phase == EMJPhase::Edit && !Lv.Note.IsEmpty())
	{
		const FSlateFontInfo F = P.Font(17.f, TEXT("Regular"));
		const float TW = P.Measure(Lv.Note, F).X;
		const FVector2f Pos(24.f, L.PanelY - 56.f), Size(TW + 150.f, 40.f);
		P.Box(Pos, Size, Hex(0x0B0C10, 0.88f), 8.f);
		P.Box(Pos, FVector2f(6.f, Size.Y), ColAmber, 3.f);
		P.Text(TEXT("STUDIO NOTE"), Pos + FVector2f(18.f, 20.f), P.Font(12.f), ColAmber, 0.f, 0.5f);
		P.Text(Lv.Note, Pos + FVector2f(126.f, 20.f), F, ColText, 0.f, 0.5f);
	}

	if (D->Phase == EMJPhase::Screening && D->bIntro && D->PhaseTime < D->IntroHold)
	{
		// a new reel opens with its own card: the name of the reel and the tool it adds
		static const TCHAR* Names[3] = { TEXT("THE HEIST"), TEXT("THE GETAWAY"), TEXT("THE SHOWDOWN") };
		static const TCHAR* Tools[3] = { TEXT("YOUR TOOL:  CUT"), TEXT("NEW TOOL:  SPLICE"), TEXT("NEW TOOL:  RESTORE DELETED SCENES") };
		const int32 R = FMath::Clamp(Lv.Reel - 1, 0, 2);
		const float A = FMath::Clamp(D->PhaseTime / 0.25f, 0.f, 1.f) * FMath::Clamp((D->IntroHold - D->PhaseTime) / 0.45f, 0.f, 1.f);
		P.Box(FVector2f(0.f, 74.f), FVector2f(L.W, L.PanelY - 100.f), Hex(0x05060A, 0.86f * A));
		P.Text(FString::Printf(TEXT("REEL %d"), Lv.Reel), Mid + FVector2f(0.f, -118.f), P.Font(32.f), Fade(ColAmber, A), 0.5f, 0.5f);
		P.Text(Names[R], Mid + FVector2f(0.f, -24.f), P.Font(124.f, TEXT("Black")), Fade(ColText, A), 0.5f, 0.5f);
		P.Text(Tools[R], Mid + FVector2f(0.f, 84.f), P.Font(24.f), Fade(ColGreen, A), 0.5f, 0.5f);
	}

	if (D->Phase == EMJPhase::Edit && D->HintTime > 0.f && !D->HintText.IsEmpty())
	{
		const float A = FMath::Clamp(D->HintTime / 0.5f, 0.f, 1.f);
		const FSlateFontInfo F = P.Font(19.f);
		const float TW = P.Measure(D->HintText, F).X;
		const FVector2f Size(TW + 110.f, 44.f), Pos(L.W - 24.f - Size.X, L.PanelY - 60.f);
		P.Box(Pos, Size, Hex(0x0B1622, 0.94f * A), 8.f);
		P.Box(Pos, FVector2f(6.f, Size.Y), Fade(Hex(0x5CC8FF), A), 3.f);
		P.Text(TEXT("HINT"), Pos + FVector2f(18.f, 22.f), P.Font(13.f), Fade(Hex(0x5CC8FF), A), 0.f, 0.5f);
		P.Text(D->HintText, Pos + FVector2f(78.f, 22.f), F, Fade(ColText, A), 0.f, 0.5f);
	}

	if (D->Phase == EMJPhase::Failed)
	{
		const float K = FMath::Clamp(D->PhaseTime / 0.18f, 0.f, 1.f);
		const float Scale = FMath::Lerp(2.2f, 1.f, K * K);
		P.Box(FVector2f(0.f, 0.f), FVector2f(L.W, L.PanelY), Hex(0x200000, 0.35f * K));
		const FSlateFontInfo F = P.Font(150.f * Scale, TEXT("Black"));
		const FVector2f Sz = P.Measure(TEXT("CUT!"), F);
		P.FrameRot(Mid + FVector2f(0.f, -40.f), Sz + FVector2f(70.f, 10.f) * Scale, Fade(ColRed, K), 14.f, 9.f * Scale, -7.f);
		P.TextRot(TEXT("CUT!"), Mid + FVector2f(0.f, -40.f), F, Fade(ColRed, K), -7.f);
		const FString Why = MJ::OutcomeText(Lv, D->Sim);
		const FSlateFontInfo WF = P.Font(26.f);
		const float WW = P.Measure(Why, WF).X + 56.f;
		P.Box(Mid + FVector2f(-WW * 0.5f, 92.f), FVector2f(WW, 52.f), Hex(0x0B0C10, 0.92f * K), 10.f);
		P.Text(Why, Mid + FVector2f(0.f, 118.f), WF, Fade(ColText, K), 0.5f, 0.5f);
		if (D->bIntro)
		{
			P.Text(TEXT("THAT WAS THE STUDIO'S THEATRICAL CUT"), Mid + FVector2f(0.f, -178.f), P.Font(18.f), Fade(ColAmber, K), 0.5f, 0.5f);
			Button(P, TEXT("fix"), Mid + FVector2f(-150.f, 162.f), FVector2f(300.f, 56.f), TEXT("FIX IT IN POST"), ColAmber, true, TEXT("SPACE"));
		}
		else
		{
			const TCHAR* Tip = D->FailCount >= 2 ? TEXT("Click or press SPACE to keep editing.   Stuck? Press H for a hint.") : TEXT("Click or press SPACE to get back to the cutting room");
			P.Text(Tip, Mid + FVector2f(0.f, 170.f), P.Font(16.f, TEXT("Regular")), Fade(D->FailCount >= 2 ? ColText : ColDim, K), 0.5f, 0.5f);
		}
	}
	else if (D->Phase == EMJPhase::Printed)
	{
		const float K = FMath::Clamp(D->PhaseTime / 0.2f, 0.f, 1.f);
		const float Scale = FMath::Lerp(2.0f, 1.f, K * K);
		P.Box(FVector2f(0.f, 0.f), FVector2f(L.W, L.PanelY), Hex(0x000000, 0.45f * K));
		const FSlateFontInfo F = P.Font(120.f * Scale, TEXT("Black"));
		const FVector2f Sz = P.Measure(TEXT("PRINT IT!"), F);
		P.FrameRot(Mid + FVector2f(0.f, -120.f), Sz + FVector2f(70.f, 10.f) * Scale, Fade(ColGreen, K), 14.f, 8.f * Scale, -4.f);
		P.TextRot(TEXT("PRINT IT!"), Mid + FVector2f(0.f, -120.f), F, Fade(ColGreen, K), -4.f);

		const FString Labels[3] = {
			TEXT("Scene printed"),
			FString::Printf(TEXT("Par: %d edit%s"), Lv.Par, Lv.Par == 1 ? TEXT("") : TEXT("s")),
			Lv.Bonus != EMJBonus::None ? Lv.BonusText : FString(TEXT("-")) };
		const bool Got[3] = { true, D->bStarPar, D->bStarBonus };
		const int32 NumStars = Lv.Bonus != EMJBonus::None ? 3 : 2;
		for (int32 i = 0; i < NumStars; ++i)
		{
			const float Delay = 0.25f + i * 0.22f;
			const float SK = FMath::Clamp((D->PhaseTime - Delay) / 0.18f, 0.f, 1.f);
			const FVector2f C = Mid + FVector2f((i - (NumStars - 1) * 0.5f) * 250.f, 10.f);
			P.Star(C, 46.f, Hex(0x2A2D36, K));
			if (Got[i] && SK > 0.f) P.Star(C, 46.f * FMath::Lerp(1.8f, 1.f, SK), Fade(ColAmber, SK));
			P.Text(Labels[i], C + FVector2f(0.f, 66.f), P.Font(16.f, TEXT("Regular")), Fade(Got[i] ? ColText : ColDim, K), 0.5f, 0.5f);
		}
		const bool bLast = !D->HasNextLevel();
		Button(P, TEXT("next"), Mid + FVector2f(-250.f, 128.f), FVector2f(280.f, 56.f), bLast ? TEXT("ROLL CREDITS") : TEXT("NEXT SCENE"), ColAmber, true, TEXT("SPACE"));
		Button(P, TEXT("keep"), Mid + FVector2f(50.f, 128.f), FVector2f(200.f, 56.f), TEXT("KEEP EDITING"), Hex(0x343946), true, TEXT("ESC"));
	}
}

int32 SMJHud::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	Hots.Reset();
	const AMJDirector* D = Director.Get();
	if (!D || D->Edit.Tracks.Num() != D->Level.Actors.Num()) return LayerId;

	const FLayout L = ComputeLayout(AllottedGeometry);
	FMJPaint P{ AllottedGeometry, OutDrawElements, LayerId + 1, L.S };
	if (D->Phase == EMJPhase::Title) PaintTitle(P, L);
	else if (D->Phase == EMJPhase::Credits) PaintCredits(P, L);
	else
	{
		PaintTopBar(P, L);
		PaintPanel(P, L);
		++P.Layer;
		PaintOverlays(P, L);
	}
	if (D->Menu != EMJMenu::None)
	{
		// buttons drawn underneath a menu must not take clicks
		for (FHot& H : Hots) H.bEnabled = false;
		++P.Layer;
		bInMenuPaint = true;
		PaintMenu(P, L);
		bInMenuPaint = false;
	}
	return P.Layer + 1;
}

// ---------------------------------------------------------------------------------------------------------------------
// title, menus, credits

bool SMJHud::IsModal() const
{
	const AMJDirector* D = Director.Get();
	return D && (D->Menu != EMJMenu::None || D->Phase == EMJPhase::Title || D->Phase == EMJPhase::Credits);
}

void SMJHud::PaintStars(FMJPaint& P, FVector2f Center, float Radius, float Gap, int32 Mask, bool bHasBonus) const
{
	const int32 Count = bHasBonus ? 3 : 2;
	for (int32 i = 0; i < Count; ++i)
	{
		const FVector2f C = Center + FVector2f((i - (Count - 1) * 0.5f) * Gap, 0.f);
		P.Star(C, Radius, (Mask & (1 << i)) ? ColAmber : Hex(0x343946));
	}
}

void SMJHud::PaintTitle(FMJPaint& P, const FLayout& L) const
{
	const AMJDirector* D = Director.Get();
	const float K = FMath::Clamp(D->PhaseTime / 0.6f, 0.f, 1.f);

	// a dark band down the left so the title reads over the desk
	TArray<FSlateGradientStop> Stops;
	Stops.Add(FSlateGradientStop(FVector2f(0.f, 0.f), Hex(0x05060A, 0.94f)));
	Stops.Add(FSlateGradientStop(FVector2f(780.f * L.S, 0.f), Hex(0x05060A, 0.80f)));
	Stops.Add(FSlateGradientStop(FVector2f(1250.f * L.S, 0.f), Hex(0x05060A, 0.f)));
	FSlateDrawElement::MakeGradient(P.Out, P.Layer, P.Geo(FVector2f(0.f, 0.f), FVector2f(1250.f, 1080.f)), Stops, Orient_Vertical);

	const float X = 96.f;
	P.Text(TEXT("A DIRECTOR'S CUT PUZZLE"), FVector2f(X + 4.f, 200.f), P.Font(20.f), Fade(ColAmber, K));
	P.Text(TEXT("FIX IT"), FVector2f(X, 226.f), P.Font(150.f, TEXT("Black")), Fade(ColText, K));
	P.Text(TEXT("IN POST"), FVector2f(X, 372.f), P.Font(150.f, TEXT("Black")), Fade(ColAmber, K));
	// a strip of film under the title
	P.Box(FVector2f(X, 548.f), FVector2f(640.f, 30.f), Hex(0x000000, K));
	for (float SX = X + 8.f; SX < X + 628.f; SX += 22.f) P.Box(FVector2f(SX, 557.f), FVector2f(11.f, 12.f), Fade(ColPaper, 0.85f * K), 2.f);

	const FSlateFontInfo Body = P.Font(21.f, TEXT("Regular"));
	P.Text(TEXT("The studio butchered your spy movie."), FVector2f(X + 4.f, 604.f), Body, Fade(ColText, K));
	P.Text(TEXT("Sneak into the cutting room and recut it before the premiere."), FVector2f(X + 4.f, 634.f), Body, Fade(ColText, K));

	const bool bStarted = D->TotalStars() > 0;
	Button(P, TEXT("play"), FVector2f(X, 708.f), FVector2f(360.f, 68.f), bStarted ? TEXT("CONTINUE") : TEXT("START THE CUT"), ColAmber, true, TEXT("SPACE"));
	Button(P, TEXT("scenes"), FVector2f(X, 792.f), FVector2f(360.f, 52.f), TEXT("SCENES"), Hex(0x343946), true);
	Button(P, TEXT("full"), FVector2f(X, 856.f), FVector2f(174.f, 52.f), TEXT("FULLSCREEN"), Hex(0x343946), true, TEXT("F11"));
	Button(P, TEXT("quit"), FVector2f(X + 186.f, 856.f), FVector2f(174.f, 52.f), TEXT("QUIT"), Hex(0x343946), true);

	P.Star(FVector2f(X + 400.f + 16.f, 742.f), 15.f, ColAmber);
	P.Text(FString::Printf(TEXT("%d / %d"), D->TotalStars(), D->MaxStars()), FVector2f(X + 440.f, 742.f), P.Font(24.f), ColText, 0.f, 0.5f);

	P.Text(TEXT("Made for the 2026 Epic AI MegaJam.  Theme: Director's Cut."), FVector2f(X + 4.f, 1010.f), P.Font(14.f, TEXT("Regular")), ColDim);
	P.Text(TEXT("Mouse to edit.  SPACE rolls film."), FVector2f(X + 4.f, 1032.f), P.Font(14.f, TEXT("Regular")), ColDim);
}

void SMJHud::PaintCredits(FMJPaint& P, const FLayout& L) const
{
	const AMJDirector* D = Director.Get();
	const float K = FMath::Clamp(D->PhaseTime / 0.5f, 0.f, 1.f);
	P.Box(FVector2f(0.f, 0.f), FVector2f(L.W, 1080.f), Hex(0x05060A, 0.80f * K));
	const FVector2f Mid(L.W * 0.5f, 0.f);
	const FSlateFontInfo F = P.Font(110.f, TEXT("Black"));
	const FVector2f Sz = P.Measure(TEXT("THAT'S A WRAP"), F);
	P.FrameRot(Mid + FVector2f(0.f, 220.f), Sz + FVector2f(80.f, 16.f), Fade(ColAmber, K), 14.f, 8.f, -3.f);
	P.TextRot(TEXT("THAT'S A WRAP"), Mid + FVector2f(0.f, 220.f), F, Fade(ColAmber, K), -3.f);
	P.Text(TEXT("The director's cut is in the can. The studio never has to know."), Mid + FVector2f(0.f, 340.f), P.Font(26.f, TEXT("Regular")), Fade(ColText, K), 0.5f, 0.5f);

	P.Star(Mid + FVector2f(-70.f, 420.f), 26.f, ColAmber);
	P.Text(FString::Printf(TEXT("%d / %d"), D->TotalStars(), D->MaxStars()), Mid + FVector2f(-30.f, 420.f), P.Font(40.f), ColText, 0.f, 0.5f);
	if (D->TotalStars() < D->MaxStars()) P.Text(TEXT("Every scene has a par and most have a studio note. Recut them from SCENES."), Mid + FVector2f(0.f, 474.f), P.Font(17.f, TEXT("Regular")), ColDim, 0.5f, 0.5f);

	static const TCHAR* Lines[] = {
		TEXT("FIX IT IN POST"),
		TEXT("Directed by Shane"),
		TEXT("Design, code, levels and edit: Claude (Anthropic), working in Unreal Engine 5.8 through Unreal MCP"),
		TEXT("Figurines and props: generated with Meshy      Audio: generated with fal"),
		TEXT("No actors were harmed. Several were cut."),
	};
	float Y = 560.f;
	for (int32 i = 0; i < UE_ARRAY_COUNT(Lines); ++i)
	{
		P.Text(Lines[i], Mid + FVector2f(0.f, Y), P.Font(i == 0 ? 24.f : 18.f, i == 0 ? TEXT("Bold") : TEXT("Regular")), Fade(i == 0 ? ColAmber : ColText, K), 0.5f, 0.5f);
		Y += i == 0 ? 46.f : 34.f;
	}
	Button(P, TEXT("title"), Mid + FVector2f(-300.f, 800.f), FVector2f(290.f, 60.f), TEXT("MAIN MENU"), ColAmber, true, TEXT("SPACE"));
	Button(P, TEXT("scenes"), Mid + FVector2f(10.f, 800.f), FVector2f(290.f, 60.f), TEXT("SCENES"), Hex(0x343946), true);
}

void SMJHud::PaintMenu(FMJPaint& P, const FLayout& L) const
{
	const AMJDirector* D = Director.Get();
	P.Box(FVector2f(0.f, 0.f), FVector2f(L.W, 1080.f), Hex(0x05060A, 0.78f));
	const FVector2f Mid(L.W * 0.5f, 540.f);

	if (D->Menu == EMJMenu::Pause)
	{
		const FVector2f Size(520.f, 640.f), Pos = Mid - Size * 0.5f;
		P.Box(Pos + FVector2f(0.f, 8.f), Size, Hex(0x000000, 0.5f), 16.f);
		P.Box(Pos, Size, Hex(0x14171F), 16.f);
		P.Box(Pos, FVector2f(Size.X, 6.f), ColAmber, 3.f);
		P.Text(TEXT("PAUSED"), Pos + FVector2f(Size.X * 0.5f, 52.f), P.Font(48.f, TEXT("Black")), ColText, 0.5f, 0.5f);

		const float BX = Pos.X + 60.f, BW = Size.X - 120.f;
		float Y = Pos.Y + 100.f;
		Button(P, TEXT("resume"), FVector2f(BX, Y), FVector2f(BW, 58.f), TEXT("BACK TO THE CUTTING ROOM"), ColAmber, true, TEXT("ESC")); Y += 72.f;
		Button(P, TEXT("restart"), FVector2f(BX, Y), FVector2f(BW, 50.f), TEXT("REWATCH THE THEATRICAL CUT"), Hex(0x343946), D->Phase != EMJPhase::Title); Y += 62.f;
		Button(P, TEXT("scenes"), FVector2f(BX, Y), FVector2f(BW, 50.f), TEXT("SCENES"), Hex(0x343946), true); Y += 62.f;
		Button(P, TEXT("title"), FVector2f(BX, Y), FVector2f(BW, 50.f), TEXT("MAIN MENU"), Hex(0x343946), true); Y += 78.f;

		for (int32 Row = 0; Row < 2; ++Row)
		{
			const bool bMusic = Row == 0;
			const float V = D->Save ? (bMusic ? D->Save->MusicVolume : D->Save->SfxVolume) : 0.f;
			P.Text(bMusic ? TEXT("MUSIC") : TEXT("SOUND"), FVector2f(BX, Y + 20.f), P.Font(16.f), ColDim, 0.f, 0.5f);
			Button(P, bMusic ? TEXT("music-") : TEXT("sfx-"), FVector2f(BX + 96.f, Y), FVector2f(40.f, 40.f), TEXT("-"), Hex(0x343946), V > 0.01f);
			for (int32 i = 0; i < 10; ++i) P.Box(FVector2f(BX + 146.f + i * 19.f, Y + 8.f), FVector2f(14.f, 24.f), (i + 0.5f) / 10.f < V ? ColAmber : Hex(0x2A2D36), 3.f);
			Button(P, bMusic ? TEXT("music+") : TEXT("sfx+"), FVector2f(BX + 346.f, Y), FVector2f(40.f, 40.f), TEXT("+"), Hex(0x343946), V < 0.99f);
			Y += 52.f;
		}
		Y += 10.f;
		Button(P, TEXT("full"), FVector2f(BX, Y), FVector2f(BW * 0.5f - 6.f, 46.f), TEXT("FULLSCREEN"), Hex(0x343946), true, TEXT("F11"));
		Button(P, TEXT("quit"), FVector2f(BX + BW * 0.5f + 6.f, Y), FVector2f(BW * 0.5f - 6.f, 46.f), TEXT("QUIT GAME"), Hex(0x5A2A2A), true);
		return;
	}

	// ---- scene select: one column per reel
	static const TCHAR* ReelNames[3] = { TEXT("REEL 1   THE HEIST"), TEXT("REEL 2   THE GETAWAY"), TEXT("REEL 3   THE SHOWDOWN") };
	const TArray<FMJLevelDef>& All = MJ::Levels();
	P.Text(TEXT("SCENES"), FVector2f(Mid.X, 86.f), P.Font(54.f, TEXT("Black")), ColText, 0.5f, 0.5f);
	P.Star(FVector2f(Mid.X - 60.f, 146.f), 13.f, ColAmber);
	P.Text(FString::Printf(TEXT("%d / %d"), D->TotalStars(), D->MaxStars()), FVector2f(Mid.X - 36.f, 146.f), P.Font(22.f), ColText, 0.f, 0.5f);

	const float ColW = FMath::Min(430.f, (L.W - 120.f) / 3.f), CardW = ColW - 24.f, X0 = Mid.X - ColW * 1.5f;
	int32 RowInReel[3] = { 0, 0, 0 };
	for (int32 r = 0; r < 3; ++r) P.Text(ReelNames[r], FVector2f(X0 + r * ColW + 12.f, 200.f), P.Font(17.f), ColAmber);
	for (int32 i = 0; i < All.Num(); ++i)
	{
		const FMJLevelDef& Lv = All[i];
		const int32 r = FMath::Clamp(Lv.Reel - 1, 0, 2);
		const FVector2f Pos(X0 + r * ColW + 12.f, 234.f + RowInReel[r]++ * 92.f), Size(CardW, 80.f);
		const int32 Mask = D->StarMask(Lv.Id);
		const bool bHover = Mouse.X >= Pos.X && Mouse.Y >= Pos.Y && Mouse.X <= Pos.X + Size.X && Mouse.Y <= Pos.Y + Size.Y;
		FHot H; H.Id = FName(*FString::Printf(TEXT("scene_%d"), i)); H.Pos = Pos; H.Size = Size; H.bEnabled = true;
		Hots.Add(H);
		P.Box(Pos + FVector2f(0.f, 4.f), Size, Hex(0x000000, 0.45f), 10.f);
		P.Box(Pos, Size, bHover ? Hex(0x2A2F3C) : Hex(0x1A1D25), 10.f);
		if (i == D->LevelIndex && D->Phase != EMJPhase::Title) P.Frame(Pos, Size, ColAmber, 10.f, 2.f);
		P.Box(Pos, FVector2f(7.f, Size.Y), (Mask & 1) ? ColGreen : Hex(0x343946), 3.f);
		P.Text(FString::Printf(TEXT("SCENE %d"), i + 1), Pos + FVector2f(22.f, 20.f), P.Font(12.f), ColDim, 0.f, 0.5f);
		P.Text(Lv.Title.ToUpper(), Pos + FVector2f(22.f, 48.f), P.Font(22.f), ColText, 0.f, 0.5f);
		PaintStars(P, Pos + FVector2f(Size.X - 62.f, 40.f), 13.f, 32.f, Mask, Lv.Bonus != EMJBonus::None);
	}
	if (RowInReel[2] == 0) P.Text(TEXT("still in the edit..."), FVector2f(X0 + 2 * ColW + 12.f, 250.f), P.Font(16.f, TEXT("Italic")), ColDim);
	Button(P, TEXT("resume"), FVector2f(Mid.X - 150.f, 990.f), FVector2f(300.f, 54.f), TEXT("BACK"), Hex(0x343946), true, TEXT("ESC"));
}