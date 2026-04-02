// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "GameplayEffectTypes.h"
#include "Animation/AnimInstance.h"
#include "AnimInstanceWithTags.generated.h"

#define UE_API OWNFRAMEWORK_API

class UAbilitySystemComponent;

/*
 * 可以映射到蓝图变量的游戏标签。变量将随着标签的添加或删除而自动更新。应该使用这些标签，而不是手动查询游戏标签。
 */
UCLASS(MinimalAPI)
class UAnimInstanceWithTags : public UAnimInstance
{
	GENERATED_BODY()

public:
	//自动注册Tag与变量之间的映射,如果有ASC的情况下
	virtual void NativeInitializeAnimation() override;
	virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override; //数据验证
#endif

protected:
	//绑定Tag与变量之间的映射
	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;
};

#undef UE_API
