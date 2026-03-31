// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "ModularMessagingSubsystem.h"
#include "UObject/Object.h"
#include "ModularGameDialogDescriptor.generated.h"

#define UE_API MODULARGAME_API

USTRUCT(BlueprintType)
struct FConfirmationDialogAction
{
	GENERATED_BODY()

public:
	/** 必填项：要提供的对话框选项。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EModularMessagingResult Result = EModularMessagingResult::Unknown;

	/** （可选）要用于代替与结果关联的操作名称的显示文本。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText OptionalDisplayText;

	bool operator==(const FConfirmationDialogAction& Other) const
	{
		return Result == Other.Result &&
			OptionalDisplayText.EqualTo(Other.OptionalDisplayText);
	}
};

/**
 * 
 */
UCLASS()
class UModularGameDialogDescriptor : public UObject
{
	GENERATED_BODY()

public:
	static UE_API UModularGameDialogDescriptor* CreateConfirmationOk(const FText& Header, const FText& Body);
	static UE_API UModularGameDialogDescriptor* CreateConfirmationOkCancel(const FText& Header, const FText& Body);
	static UE_API UModularGameDialogDescriptor* CreateConfirmationYesNo(const FText& Header, const FText& Body);
	static UE_API UModularGameDialogDescriptor* CreateConfirmationYesNoCancel(const FText& Header, const FText& Body);

public:
	/** 要显示的消息标题*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Header;

	/** 要显示的消息正文*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Body;

	/** 确认按钮的输入操作。 */
	UPROPERTY(BlueprintReadWrite)
	TArray<FConfirmationDialogAction> ButtonActions;
};

UCLASS(MinimalAPI, Abstract)
class UModularGameDialog : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UE_API UModularGameDialog();

	UE_API virtual void SetupDialog(UModularGameDialogDescriptor* Descriptor, FModularMessagingResultDelegate ResultCallback);

	UE_API virtual void KillDialog();
};

#undef UE_API
