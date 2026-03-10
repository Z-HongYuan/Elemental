// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ModularPlayerController.generated.h"

#define UE_API MODULARACTORS_API

/**
 * 自动注册接受者的玩家控制器,允许添加或移除组件或者其他Actions
 * 支持模块化的最小类
 */
UCLASS(MinimalAPI, Blueprintable, Abstract, meta=(
	DisplayName="Modular Player Controller",
	ShortTooltip="自动注册接收者的模块化玩家控制器，支持动态组件管理",
	Category="Modular Actor"
))
class AModularPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface

	//~ Begin APlayerController interface
	UE_API virtual void ReceivedPlayer() override;
	UE_API virtual void PlayerTick(float DeltaTime) override;
	//~ End APlayerController interface
};

#undef UE_API
