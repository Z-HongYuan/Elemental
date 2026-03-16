// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ModularPlayerState.generated.h"

#define UE_API MODULARACTORS_API

/**
 * 自动注册接受者的玩家状态,允许添加或移除组件或者其他Actions
 * 支持模块化的最小类
 */
UCLASS(MinimalAPI, Blueprintable, Abstract, meta=(
	DisplayName="Modular Player State",
	ShortTooltip="自动注册接收者的模块化玩家状态，支持动态组件管理",
	Category="Modular Actor"
))
class AModularPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void Reset() override;
	//~ End AActor interface

protected:
	//~ Begin APlayerState interface
	UE_API virtual void CopyProperties(APlayerState* PlayerState) override;
	//~ End APlayerState interface
};

#undef UE_API
