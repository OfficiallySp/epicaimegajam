// Fix It In Post - the whole in-game UI as one immediate-mode Slate leaf widget (custom painted, own hit testing).
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

class AMJDirector;
struct FMJPaint;
struct FMJClip;

class SMJHud : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SMJHud) {}
		SLATE_ARGUMENT(TWeakObjectPtr<AMJDirector>, Director)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(1920.0, 1080.0); }
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;

private:
	/** Everything is laid out on a virtual canvas 1080 units tall; the width follows the aspect ratio. */
	struct FLayout
	{
		float S = 1.f;       // virtual -> local scale
		float W = 1920.f;    // virtual width
		float PanelY = 0.f, PanelH = 0.f;
		float ToolbarY = 0.f, RulerY = 0.f, RowY0 = 0.f, RowH = 78.f;
		float X0 = 0.f, X1 = 0.f, BeatW = 80.f;
		float BinX = 0.f, BinW = 0.f;
		int32 NumTracks = 0;
		int32 Beats = 0;
	};

	struct FHot
	{
		FName Id;
		FVector2f Pos, Size;
		bool bEnabled = true;
	};

	enum class EDrag : uint8 { None, Clip, Bin, Scrub };

	FLayout ComputeLayout(const FGeometry& Geo) const;
	FVector2f ToVirtual(const FGeometry& Geo, const FPointerEvent& E) const;
	bool ClipRect(const FLayout& L, int32 Track, int32 Index, FVector2f& OutPos, FVector2f& OutSize) const;
	bool HitClip(const FLayout& L, FVector2f P, int32& OutTrack, int32& OutIndex) const;
	int32 HitBin(const FLayout& L, FVector2f P) const;
	bool BinRect(const FLayout& L, int32 BinIndex, FVector2f& OutPos, FVector2f& OutSize) const;
	int32 TrackAt(const FLayout& L, float Y, float Slack) const;
	int32 DropIndex(const FLayout& L, int32 Track, float X, int32 SkipUid, float* OutCaretX = nullptr) const;
	FName HotAt(FVector2f P) const;
	void Click(FName Id);
	void CutSelection();

	void PaintTopBar(FMJPaint& P, const FLayout& L) const;
	void PaintPanel(FMJPaint& P, const FLayout& L) const;
	void PaintClip(FMJPaint& P, const FMJClip& C, int32 OwnerTrack, FVector2f Pos, FVector2f Size, bool bLocked, bool bSelected, bool bHover, float Alpha, bool bPlaying) const;
	void PaintOverlays(FMJPaint& P, const FLayout& L) const;
	void PaintTitle(FMJPaint& P, const FLayout& L) const;
	void PaintCredits(FMJPaint& P, const FLayout& L) const;
	void PaintMenu(FMJPaint& P, const FLayout& L) const;
	void PaintStars(FMJPaint& P, FVector2f Center, float Radius, float Gap, int32 Mask, bool bHasBonus) const;
	bool IsModal() const;
	bool Button(FMJPaint& P, FName Id, FVector2f Pos, FVector2f Size, const FString& Label, const FLinearColor& Color, bool bEnabled, const FString& Key = FString()) const;

	TWeakObjectPtr<AMJDirector> Director;

	mutable TArray<FHot> Hots;
	mutable bool bInMenuPaint = false;
	FVector2f Mouse = FVector2f(-1000.f, -1000.f);
	FName PressedHot;
	EDrag Drag = EDrag::None;
	bool bDragMoved = false;
	FVector2f DragStart = FVector2f::ZeroVector;
	FVector2f DragGrab = FVector2f::ZeroVector; // offset of the grab point inside the clip
	int32 DragTrack = -1, DragIndex = -1, DragUid = -1;
};
