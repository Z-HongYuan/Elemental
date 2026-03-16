// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "GameFramework/GameMode.h"
#include "ModularGameMode.generated.h"

#define UE_API MODULARACTORS_API

/*
 * 默认参数为模块化对象的GameMode
 */
UCLASS(MinimalAPI, Blueprintable, Abstract, meta=(
	DisplayName="Modular Game Mode Base",
	ShortTooltip="默认使用模块化对象的GameMode",
	Category="Modular Actor"
))
class AModularGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	UE_API AModularGameModeBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};


/*
 * 默认参数为模块化对象的GameMode
 */
UCLASS(MinimalAPI, Blueprintable, Abstract, meta=(
	DisplayName="Modular Game Mode",
	ShortTooltip="默认使用模块化对象的GameMode",
	Category="Modular Actor"
))
class AModularGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	UE_API AModularGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

#undef UE_API
