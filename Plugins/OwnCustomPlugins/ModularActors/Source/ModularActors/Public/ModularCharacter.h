// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ModularCharacter.generated.h"

#define UE_API MODULARACTORS_API

/*
 * 自动注册接受者的角色,允许添加或移除组件或者其他Actions
 * 支持模块化的最小类
 */
UCLASS(MinimalAPI, Blueprintable, Abstract, meta=(
	DisplayName="Modular Character",
	ShortTooltip="自动注册接收者的模块化角色，支持动态组件管理",
	Category="Modular Actor"
))
class AModularCharacter : public ACharacter
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
