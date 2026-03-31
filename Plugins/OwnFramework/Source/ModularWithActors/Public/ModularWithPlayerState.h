// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerState.h"
#include "ModularWithPlayerState.generated.h"

#define UE_API MODULARWITHACTORS_API

/* 支持游戏功能插件扩展的最小玩家状态类,将在游戏过程中注册为模块化行为接受者*/
UCLASS(MinimalAPI, Blueprintable)
class AModularWithPlayerState : public APlayerState
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
