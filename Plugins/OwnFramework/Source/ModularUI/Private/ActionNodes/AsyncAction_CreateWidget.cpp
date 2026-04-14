// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ActionNodes/AsyncAction_CreateWidget.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/AssetManager.h"
#include "System/ModularUIHelperFunction.h"

UAsyncAction_CreateWidget* UAsyncAction_CreateWidget::CreateWidgetAsync(UObject* InWorldContextObject,
                                                                        TSoftClassPtr<UUserWidget> InUserWidgetSoftClass,
                                                                        APlayerController* InOwningPlayer,
                                                                        bool bSuspendInputUntilComplete)
{
	//检查参数是否有效
	if (InUserWidgetSoftClass.IsNull())
	{
		FFrame::KismetExecutionMessage(TEXT("CreateWidgetAsync was passed a null UserWidgetSoftClass"), ELogVerbosity::Error);
		return nullptr;
	}

	//InWorldContextObject->GetWorld();
	UWorld* World = GEngine->GetWorldFromContextObject(InWorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		FFrame::KismetExecutionMessage(TEXT("CreateWidgetAsync was passed a null World"), ELogVerbosity::Error);
		return nullptr;
	}

	UAsyncAction_CreateWidget* Action = NewObject<UAsyncAction_CreateWidget>();
	Action->UserWidgetSoftClass = InUserWidgetSoftClass;
	Action->OwningPlayer = InOwningPlayer;
	Action->World = World;
	Action->GameInstance = World->GetGameInstance();
	Action->bSuspendInputUntilComplete = bSuspendInputUntilComplete;

	Action->RegisterWithGameInstance(World);

	return Action;
}

void UAsyncAction_CreateWidget::Activate()
{
	SuspendInputToken = bSuspendInputUntilComplete ? UModularUIHelperFunction::SuspendInputForPlayer(OwningPlayer.Get(), InputFilterReason_Template) : NAME_None;

	//TWeakObjectPtr<UAsyncAction_CreateWidget> LocalWeakThis(this);直接绑定的成员函数,所以不需要使用自身的弱引用
	StreamingHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		UserWidgetSoftClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnWidgetLoaded),
		FStreamableManager::AsyncLoadHighPriority
	);

	// 设置Node的取消的调用
	StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(
			this, [this]()
			{
				UModularUIHelperFunction::ResumeInputForPlayer(OwningPlayer.Get(), SuspendInputToken);
			})
	);
}

void UAsyncAction_CreateWidget::Cancel()
{
	Super::Cancel();

	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}
}

void UAsyncAction_CreateWidget::OnWidgetLoaded()
{
	if (bSuspendInputUntilComplete)
	{
		UModularUIHelperFunction::ResumeInputForPlayer(OwningPlayer.Get(), SuspendInputToken);
	}

	// 如果加载成功了就直接创建这个控件,并且广播这个控件的引用
	TSubclassOf<UUserWidget> UserWidgetClass = UserWidgetSoftClass.Get();
	if (UserWidgetClass)
	{
		UUserWidget* UserWidget = UWidgetBlueprintLibrary::Create(World.Get(), UserWidgetClass, OwningPlayer.Get());
		OnCompleted.Broadcast(UserWidget);
	}

	StreamingHandle.Reset();

	SetReadyToDestroy();
}
