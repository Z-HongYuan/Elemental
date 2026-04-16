// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ActionNodes/AsyncAction_PushWidgetToLayerForPlayer.h"

#include "Engine/StreamableManager.h"
#include "Widgets/ModularRootLayoutWidget.h"

UAsyncAction_PushWidgetToLayerForPlayer* UAsyncAction_PushWidgetToLayerForPlayer::PushWidgetToLayerForPlayer(APlayerController* InOwningPlayer,
                                                                                                              TSoftClassPtr<UCommonActivatableWidget> InWidgetClass,
                                                                                                              FGameplayTag InContainerTag,
                                                                                                              bool bSuspendInputUntilComplete)
{
	if (InWidgetClass.IsNull())
	{
		FFrame::KismetExecutionMessage(TEXT("PushContentToLayerForPlayer was passed a null WidgetClass"), ELogVerbosity::Error);
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(InOwningPlayer, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		FFrame::KismetExecutionMessage(TEXT("CreateWidgetAsync was passed a null World"), ELogVerbosity::Error);
		return nullptr;
	}

	UAsyncAction_PushWidgetToLayerForPlayer* Action = NewObject<UAsyncAction_PushWidgetToLayerForPlayer>();
	Action->WidgetClass = InWidgetClass;
	Action->OwningPlayerPtr = InOwningPlayer;
	Action->ContainerTag = InContainerTag;
	Action->bSuspendInputUntilComplete = bSuspendInputUntilComplete;

	Action->RegisterWithGameInstance(World);

	return Action;
}

void UAsyncAction_PushWidgetToLayerForPlayer::Activate()
{
	if (UModularRootLayoutWidget* RootLayout = UModularRootLayoutWidget::GetModularRootLayoutWidget(OwningPlayerPtr.Get()))
	{
		TWeakObjectPtr<UAsyncAction_PushWidgetToLayerForPlayer> WeakThis(this);
		StreamingHandle = RootLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(
			ContainerTag,
			bSuspendInputUntilComplete,
			WidgetClass,
			//状态回调函数,每次调用都会根据状态来匹配
			[this, WeakThis](const EAsyncWidgetLayerState State, UCommonActivatableWidget* Widget)
			{
				if (WeakThis.IsValid())
				{
					switch (State)
					{
					case EAsyncWidgetLayerState::Initialize:
						BeforePush.Broadcast(Widget);
						break;
					case EAsyncWidgetLayerState::AfterPush:
						AfterPush.Broadcast(Widget);
						SetReadyToDestroy();
						break;
					case EAsyncWidgetLayerState::Canceled:
						SetReadyToDestroy();
						break;
					}
				}
				//TODO 这里是否需要销毁? 因为一旦状态回调函数执行了 Initialize,就会销毁本对象,那么接下来的两次状态回调函数,会发生什么
				//按照逻辑来讲,应该在第一次分支的时候不会销毁,在第二三次分支的时候才会销毁
				//SetReadyToDestroy();
			});
	}
	else
	{
		SetReadyToDestroy();
	}
}

void UAsyncAction_PushWidgetToLayerForPlayer::Cancel()
{
	Super::Cancel();

	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}
}
