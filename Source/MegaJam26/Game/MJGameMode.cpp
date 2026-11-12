// Fix It In Post - game mode.
#include "Game/MJGameMode.h"

#include "Engine/World.h"
#include "Game/MJDirector.h"

AMJGameMode::AMJGameMode()
{
	DefaultPawnClass = nullptr;
}

void AMJGameMode::StartPlay()
{
	Super::StartPlay();
	GetWorld()->SpawnActor<AMJDirector>(FVector::ZeroVector, FRotator::ZeroRotator);
}
