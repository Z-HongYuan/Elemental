// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ModularAIController.generated.h"

#define UE_API MODULARACTORS_API

/*
 * 自动注册接受者的AI控制器,允许添加或移除组件或者其他Actions
 * 支持模块化的最小类
 */
UCLASS(MinimalAPI, Blueprintable, Abstract, meta=(
	DisplayName="Modular AI Controller",
	ShortTooltip="自动注册接收者的模块化 AI 控制器，支持动态组件管理",
	Category="Modular Actor"
))
class AModularAIController : public AAIController
{
	GENERATED_BODY()

public:
	//~ Begin AActor Interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface
};

#undef UE_API
