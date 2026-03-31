// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "GameFramework/Pawn.h"
#include "ModularWithPawn.generated.h"

#define UE_API MODULARWITHACTORS_API

/* 支持游戏功能插件扩展的最小角色类,将在游戏过程中注册为模块化行为接受者*/
UCLASS(MinimalAPI, Blueprintable)
class AModularWithPawn : public APawn
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
