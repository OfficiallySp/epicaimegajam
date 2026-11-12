// Fix It In Post - game mode: no pawn, just the director and the stage.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MJGameMode.generated.h"

UCLASS()
class MEGAJAM26_API AMJGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMJGameMode();
	virtual void StartPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override {} // there is no pawn: the director drives everything
};
