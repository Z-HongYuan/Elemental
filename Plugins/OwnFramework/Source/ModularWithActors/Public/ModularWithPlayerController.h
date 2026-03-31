// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"
#include "ModularWithPlayerController.generated.h"

#define UE_API MODULARWITHACTORS_API

/* 支持游戏功能插件扩展的最小玩家控制器类,将在游戏过程中注册为模块化行为接受者*/
UCLASS(MinimalAPI, Blueprintable)
class AModularWithPlayerController : public APlayerController
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
