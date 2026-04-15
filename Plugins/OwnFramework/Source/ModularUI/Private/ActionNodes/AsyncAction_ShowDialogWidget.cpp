// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ActionNodes/AsyncAction_ShowDialogWidget.h"

#include "Blueprint/UserWidget.h"
#include "SystemMessage/ModularGameDialogDataObj.h"

UAsyncAction_ShowDialogWidget* UAsyncAction_ShowDialogWidget::ShowConfirmationYesNo(UObject* InWorldContextObject, FText Title, FText Message)
{
	UAsyncAction_ShowDialogWidget* Action = NewObject<UAsyncAction_ShowDialogWidget>();
	Action->WorldContextObject = InWorldContextObject;
	Action->DialogDataObj = UModularGameDialogDataObj::CreateConfirmationYesNo(Title, Message);

	Action->RegisterWithGameInstance(InWorldContextObject);

	return Action;
}

UAsyncAction_ShowDialogWidget* UAsyncAction_ShowDialogWidget::ShowConfirmationOkCancel(UObject* InWorldContextObject, FText Title, FText Message)
{
	UAsyncAction_ShowDialogWidget* Action = NewObject<UAsyncAction_ShowDialogWidget>();
	Action->WorldContextObject = InWorldContextObject;
	Action->DialogDataObj = UModularGameDialogDataObj::CreateConfirmationOkCancel(Title, Message);

	Action->RegisterWithGameInstance(InWorldContextObject);

	return Action;
}

UAsyncAction_ShowDialogWidget* UAsyncAction_ShowDialogWidget::ShowConfirmationCustom(UObject* InWorldContextObject, UModularGameDialogDataObj* DialogDataObj)
{
	UAsyncAction_ShowDialogWidget* Action = NewObject<UAsyncAction_ShowDialogWidget>();
	Action->WorldContextObject = InWorldContextObject;
	Action->DialogDataObj = DialogDataObj;

	Action->RegisterWithGameInstance(InWorldContextObject);

	return Action;
}

void UAsyncAction_ShowDialogWidget::Activate()
{
	//如果世界上下文有效,但是本地玩家无效,那么就尝试用其他方法获取本地玩家
	if (WorldContextObject && !TargetLocalPlayer)
	{
		//从控件中获取本地玩家,如果是从控件中调用
		if (UUserWidget* UserWidget = Cast<UUserWidget>(WorldContextObject))
		{
			TargetLocalPlayer = UserWidget->GetOwningLocalPlayer<ULocalPlayer>();
		}
		//从玩家控制器中获取本地玩家,如果是从玩家控制器中调用
		else if (APlayerController* PC = Cast<APlayerController>(WorldContextObject))
		{
			TargetLocalPlayer = PC->GetLocalPlayer();
		}
		//从世界中获取本地玩家,如果是从世界中调用
		else if (UWorld* World = WorldContextObject->GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance<UGameInstance>())
			{
				TargetLocalPlayer = GameInstance->GetPrimaryPlayerController(false)->GetLocalPlayer(); //只返回本地控制的玩家
			}
		}
	}

	// 如果本地玩家有效,那么就尝试用本地玩家显示对话框
	if (TargetLocalPlayer)
	{
		if (UModularMessagingManager* MessagingManager = TargetLocalPlayer->GetSubsystem<UModularMessagingManager>())
		{
			//构建一个委托，用于处理结果
			FModularMessagingResultDelegate ResultCallback = FModularMessagingResultDelegate::CreateUObject(this, &UAsyncAction_ShowDialogWidget::HandleConfirmationResult);
			MessagingManager->ShowConfirmation(DialogDataObj, ResultCallback);
			return;
		}
	}

	// 如果无法确认，就处理未知结果，什么都不播
	HandleConfirmationResult(EModularMessagingResult::Unknown);
}

void UAsyncAction_ShowDialogWidget::HandleConfirmationResult(EModularMessagingResult ConfirmationResult)
{
	OnResulted.Broadcast(ConfirmationResult);

	SetReadyToDestroy();
}
