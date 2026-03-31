// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularWithGameMode.h"

#include "ModularWithGameState.h"
#include "ModularWithPawn.h"
#include "ModularWithPlayerController.h"
#include "ModularWithPlayerState.h"

AModularWithGameMode::AModularWithGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	GameStateClass = AModularWithGameStateBase::StaticClass();
	PlayerControllerClass = AModularWithPlayerController::StaticClass();
	PlayerStateClass = AModularWithPlayerState::StaticClass();
	DefaultPawnClass = AModularWithPawn::StaticClass();
}

AModularWithGameModeBase::AModularWithGameModeBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	GameStateClass = AModularWithGameStateBase::StaticClass();
	PlayerControllerClass = AModularWithPlayerController::StaticClass();
	PlayerStateClass = AModularWithPlayerState::StaticClass();
	DefaultPawnClass = AModularWithPawn::StaticClass();
}
