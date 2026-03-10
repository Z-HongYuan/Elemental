// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "GameFramework/GameState.h"
#include "ModularGameState.generated.h"

#define UE_API MODULARACTORS_API

/*
 * 自动注册接受者的游戏状态,允许添加或移除组件或者其他Actions
 * 支持模块化的最小类
 */
UCLASS(MinimalAPI, Blueprintable, Abstract, meta=(
	DisplayName="Modular Game State Base",
	ShortTooltip="自动注册接收者的模块化游戏状态，支持动态组件管理",
	Category="Modular Actor"
))
class AModularGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface
};


/*
 * 自动注册接受者的游戏状态,允许添加或移除组件或者其他Actions
 * 支持模块化的最小类
 */
UCLASS(MinimalAPI, Blueprintable, Abstract, meta=(
	DisplayName="Modular Game State",
	ShortTooltip="自动注册接收者的模块化游戏状态，支持动态组件管理",
	Category="Modular Actor"
))
class AModularGameState : public AGameState
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

#undef UE_API
