// Fix It In Post - in-game UI. One leaf widget paints everything and does its own hit testing; the director holds all state.
#include "UI/SMJHud.h"

#include "Brushes/SlateRoundedBoxBrush.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Game/MJDirector.h"
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

	const FSlateBrush* FrameBrush(float Radius, float Thick)
	{
		static TMap<int32, TSharedPtr<FSlateRoundedBoxBrush>> Cache;
		const int32 R = FMath::Clamp(FMath::RoundToInt(Radius), 0, 200);
		const int32 T = FMath::Clamp(FMath::RoundToInt(Thick * 2.f), 1, 40);
		TSharedPtr<FSlateRoundedBoxBrush>& B = Cache.FindOrAdd(R * 64 + T);
		if (!B.IsValid()) B = MakeShared<FSlateRoundedBoxBrush>(FLinearColor::Transparent, (float)R, FLinearColor::White, T * 0.5f);
		return B.Get();
	}
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
		if (Sz.X <= 0.f || Sz.Y <= 0.f) return;
		FSlateDrawElement::MakeBox(Out, Layer, Geo(P, Sz), FrameBrush(Radius * S, Thick * S), ESlateDrawEffect::None, C);
	}

	FSlateFontInfo Font(float Px, const TCHAR* Face = TEXT("Bold")) const
	{
		return FCoreStyle::GetDefaultFontStyle(Face, FMath::Max(5.f, Px * S * 0.75f));
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
		FSlateDrawElement::MakeBox(Out, Layer, PG, FrameBrush(Radius * S, Thick * S), ESlateDrawEffect::None, C);
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
	if (Id == Roll) D->Roll();
	else if (Id == Undo) D->Undo();
	else if (Id == Redo) D->Redo();
	else if (Id == Reset) D->ResetEdits();
	else if (Id == Cut) CutSelection();
	else if (Id == Next) D->NextLevel();
	else if (Id == Keep) D->BackToEdit();
	D->Sfx(TEXT("click"));
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
	if (!PressedHot.IsNone()) return Reply;

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
	if (D && D->Phase == EMJPhase::Edit && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
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

	if (K == EKeys::F) { D->bFast = true; return FReply::Handled(); }
	if (K == EKeys::PageDown && !KeyEvent.IsRepeat()) { D->NextLevel(); return FReply::Handled(); }
	if (K == EKeys::PageUp && !KeyEvent.IsRepeat()) { D->LoadLevel(D->LevelIndex - 1); return FReply::Handled(); }

	switch (D->Phase)
	{
	case EMJPhase::Edit:
		if (bGo && !KeyEvent.IsRepeat()) D->Roll();
		else if (K == EKeys::X || K == EKeys::Delete || K == EKeys::BackSpace) CutSelection();
		else if (K == EKeys::Z) D->Undo();
		else if (K == EKeys::Y) D->Redo();
		else if (K == EKeys::R && !KeyEvent.IsRepeat()) D->ResetEdits();
		else if (K == EKeys::Left) D->StepBeat(-1);
		else if (K == EKeys::Right) D->StepBeat(1);
		else if (K == EKeys::Escape) D->SelUid = -1;
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
	const bool bHover = bEnabled && Mouse.X >= Pos.X && Mouse.Y >= Pos.Y && Mouse.X <= Pos.X + Size.X && Mouse.Y <= Pos.Y + Size.Y;
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
		P.Text(TEXT("SCREENING"), FVector2f(64.f, 37.f), P.Font(18.f), ColText, 0.f, 0.5f);
		const float Secs = D->PlayTime / FMath::Max(0.1f, D->BeatsPerSecond);
		const int32 Frames = FMath::FloorToInt(Secs * 24.f);
		P.Text(FString::Printf(TEXT("%02d:%02d:%02d"), Frames / 1440, (Frames / 24) % 60, Frames % 24), FVector2f(L.W - 34.f, 37.f), P.Font(20.f, TEXT("Mono")), ColText, 1.f, 0.5f);
		P.Text(FString::Printf(TEXT("%s  -  %s"), *Lv.Title.ToUpper(), *Lv.Slug), FVector2f(L.W * 0.5f, 37.f), P.Font(16.f, TEXT("Regular")), ColDim, 0.5f, 0.5f);
		return;
	}

	P.Box(FVector2f(0.f, 0.f), FVector2f(L.W, 112.f), Hex(0x0B0C10, 0.86f));
	P.Box(FVector2f(0.f, 112.f), FVector2f(L.W, 2.f), Hex(0xFFB020, 0.5f));

	P.Text(FString::Printf(TEXT("REEL %d   SCENE %d"), Lv.Reel, D->LevelIndex + 1), FVector2f(28.f, 14.f), P.Font(14.f), ColAmber);
	P.Text(Lv.Title.ToUpper(), FVector2f(28.f, 34.f), P.Font(34.f, TEXT("Black")), ColText);
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
	P.Text(FString::Printf(TEXT("%d of %d used   par %d"), Used, Max, Lv.Par), FVector2f(RX, 76.f), P.Font(13.f, TEXT("Regular")), ColDim, 1.f, 0.f);
	if (Lv.Bonus != EMJBonus::None)
	{
		P.Star(FVector2f(RX - 372.f, 92.f), 9.f, ColAmber);
		P.Text(Lv.BonusText, FVector2f(RX - 358.f, 92.f), P.Font(13.f, TEXT("Regular")), ColText, 0.f, 0.5f);
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

	if (D->ToastTime > 0.f && !D->Toast.IsEmpty())
	{
		const float A = FMath::Clamp(D->ToastTime / 0.4f, 0.f, 1.f);
		P.Text(D->Toast, FVector2f(BX + 8.f, BY + BH * 0.5f), P.Font(18.f), Fade(ColRed, A), 0.f, 0.5f);
	}
	else if (bEdit)
	{
		FString Tools = TEXT("Tools:  CUT (X or drag off the strip)");
		if (Lv.Tools & MJTool_Move) Tools += TEXT("   SPLICE (drag along the strip)");
		if (Lv.Tools & MJTool_Insert) Tools += TEXT("   RESTORE (drag from Deleted Scenes)");
		P.Text(Tools, FVector2f(BX + 8.f, BY + BH * 0.5f), P.Font(14.f, TEXT("Regular")), ColDim, 0.f, 0.5f);
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
			const FMJActorDef& Owner = Lv.Actors[D->Edit.Bin[i].Actor];
			P.Text(TEXT("for"), FVector2f(Pos.X + CS.X + 10.f, Pos.Y + 11.f), P.Font(10.f, TEXT("Regular")), ColDim, 0.f, 0.5f);
			P.Text(Owner.Name, FVector2f(Pos.X + CS.X + 10.f, Pos.Y + 27.f), P.Font(13.f), RoleColor(Owner.Role), 0.f, 0.5f);
		}
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
		P.Text(TEXT("Click or press SPACE to get back to the cutting room"), Mid + FVector2f(0.f, 170.f), P.Font(16.f, TEXT("Regular")), Fade(ColDim, K), 0.5f, 0.5f);
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
		Button(P, TEXT("next"), Mid + FVector2f(-250.f, 128.f), FVector2f(280.f, 56.f), bLast ? TEXT("BACK TO SCENE 1") : TEXT("NEXT SCENE"), ColAmber, true, TEXT("SPACE"));
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
	PaintTopBar(P, L);
	PaintPanel(P, L);
	++P.Layer;
	PaintOverlays(P, L);
	return P.Layer + 1;
}
