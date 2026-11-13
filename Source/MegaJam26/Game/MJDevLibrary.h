// Fix It In Post - development helpers callable from editor Python (level design loop without recompiling).
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MJDevLibrary.generated.h"

UCLASS()
class MEGAJAM26_API UMJDevLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Parse level text and brute-force every level in it. Returns one report per level (and the theatrical trace if asked). */
	UFUNCTION(BlueprintCallable, Category = "MegaJam|Dev")
	static FString SolveLevels(const FString& LevelText, bool bTrace);

	/**
	 * Apply edits to one level and simulate. Edits are separated by ';':
	 *   cut <track> <index>      move <track> <from> <to>      insert <bin index> <track> <at>
	 * LevelId may be empty to use the first level in the text.
	 */
	UFUNCTION(BlueprintCallable, Category = "MegaJam|Dev")
	static FString TryEdits(const FString& LevelText, const FString& LevelId, const FString& Edits);

	/** The embedded campaign's solver report (same as the mj.SolveAll console command, but returned as text). */
	UFUNCTION(BlueprintCallable, Category = "MegaJam|Dev")
	static FString SolveCampaign();
};
