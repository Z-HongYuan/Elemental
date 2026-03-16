// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ModularPawn.generated.h"

#define UE_API MODULARACTORS_API

/*
 * 自动注册接受者的Pawn,允许添加或移除组件或者其他Actions
 * 支持模块化的最小类
 */
UCLASS(MinimalAPI, Blueprintable, Abstract, meta=(
	DisplayName="Modular Pawn",
	ShortTooltip="自动注册接收者的模块化Pawn，支持动态组件管理",
	Category="Modular Actor"
))
class AModularPawn : public APawn
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
