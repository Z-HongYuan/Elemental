// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "GameFramework/GameMode.h"
#include "ModularWithGameMode.generated.h"

#define UE_API MODULARWITHACTORS_API

/* 支持游戏功能插件扩展的最小游戏模式类,将在游戏过程中注册为模块化行为接受者 */
UCLASS(MinimalAPI, Blueprintable)
class AModularWithGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	UE_API AModularWithGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};


/* 支持游戏功能插件扩展的最小游戏模式类,将在游戏过程中注册为模块化行为接受者 */
UCLASS(MinimalAPI, Blueprintable)
class AModularWithGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	UE_API AModularWithGameModeBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
#undef UE_API
