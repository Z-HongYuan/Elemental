// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "ModularMessagingManager.generated.h"

#define UE_API MODULARUI_API

class UModularConfirmPop;
class UModularGameDialogDataObj;

/** 对话产生的结果种类 */
UENUM(BlueprintType)
enum class EModularMessagingResult : uint8
{
	/** 按下了“是”按钮*/
	Confirmed,
	/** 按下了“不”按钮 */
	Declined,
	/** 按下了“忽略/取消”按钮 */
	Cancelled,
	/** 对话被明确关闭（无用户输入） */
	Killed,
	Unknown UMETA(Hidden)
};

DECLARE_DELEGATE_OneParam(FModularMessagingResultDelegate, EModularMessagingResult /* Result */);

/*提供最基础的对话框架*/
UCLASS(MinimalAPI, Config = Game)
class UModularMessagingManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	//保证继承链单例
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// 显示确认对话框, 将会在子类中实现, 默认实现为空
	UE_API virtual void ShowConfirmation(UModularGameDialogDataObj* DialogDataObj, FModularMessagingResultDelegate ResultCallback = FModularMessagingResultDelegate());

	// 显示错误对话框, 将会在子类中实现, 默认实现为空
	UE_API virtual void ShowError(UModularGameDialogDataObj* DialogDataObj, FModularMessagingResultDelegate ResultCallback = FModularMessagingResultDelegate());
};

#undef UE_API
