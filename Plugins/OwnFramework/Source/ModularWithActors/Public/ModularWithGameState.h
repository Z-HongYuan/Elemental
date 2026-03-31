// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "GameFramework/GameState.h"
#include "ModularWithGameState.generated.h"

#define UE_API MODULARWITHACTORS_API

/* 支持游戏功能插件扩展的最小游戏模式类,将在游戏过程中注册为模块化行为接受者 */
UCLASS(MinimalAPI, Blueprintable)
class AModularWithGameState : public AGameState
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface

protected:
	//~ Begin AGameState interface
	UE_API virtual void HandleMatchHasStarted() override;
	//~ Begin AGameState interface
};


/* 支持游戏功能插件扩展的最小游戏模式类,将在游戏过程中注册为模块化行为接受者 */
UCLASS(MinimalAPI, Blueprintable)
class AModularWithGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface
};
#undef UE_API
