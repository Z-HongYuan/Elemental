// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "SystemMessage/ModularGameDialogDataObj.h"
#include "ModularGameDialog.generated.h"

#define UE_API MODULARUI_API


/*这个是最基础的对话控件,所有的对话都需要从这里继承*/
UCLASS(MinimalAPI, Abstract)
class UModularGameDialog : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	//初始化对话框 , 通过数据对象初始化
	UE_API virtual void SetupDialog(UModularGameDialogDataObj* DialogDataObj, FModularMessagingResultDelegate ResultCallback)
	{
	}

	// 关闭对话框会发生什么 API 一般是返回用户输入结果
	UE_API virtual void KillDialog()
	{
	}
};

#undef UE_API
