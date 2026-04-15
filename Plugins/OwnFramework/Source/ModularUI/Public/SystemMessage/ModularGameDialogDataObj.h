// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "ModularMessagingManager.h"
#include "UObject/Object.h"
#include "ModularGameDialogDataObj.generated.h"

#define UE_API MODULARUI_API

/* 对话中每个选项的含义 */
USTRUCT(BlueprintType)
struct FConfirmationDialogAction
{
	GENERATED_BODY()

public:
	/** 必须要更改的 选项代表的含义 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EModularMessagingResult Result = EModularMessagingResult::Unknown;

	/** 选项展示的信息文本 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText OptionalDisplayText;

	bool operator==(const FConfirmationDialogAction& Other) const
	{
		return Result == Other.Result &&
			OptionalDisplayText.EqualTo(Other.OptionalDisplayText);
	}
};

/*对于对话控件所代表的数据对象*/
UCLASS(MinimalAPI)
class UModularGameDialogDataObj : public UObject
{
	GENERATED_BODY()

public:
	/*静态函数*/
	static UE_API UModularGameDialogDataObj* CreateConfirmationOk(const FText& Title, const FText& Content);
	static UE_API UModularGameDialogDataObj* CreateConfirmationOkCancel(const FText& Title, const FText& Content);
	static UE_API UModularGameDialogDataObj* CreateConfirmationYesNo(const FText& Title, const FText& Content);
	static UE_API UModularGameDialogDataObj* CreateConfirmationYesNoCancel(const FText& Title, const FText& Content);


	/** 对话显示的标题 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/** 对话显示的内容 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Content;

	/** 对话需要使用的按钮动作信息 */
	UPROPERTY(BlueprintReadWrite)
	TArray<FConfirmationDialogAction> ButtonActions;
};

#undef UE_API
