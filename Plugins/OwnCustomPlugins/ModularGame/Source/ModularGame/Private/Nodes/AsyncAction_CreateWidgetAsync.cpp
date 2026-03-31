// Copyright © 2026 鸿源z. All Rights Reserved.


#include "Nodes/AsyncAction_CreateWidgetAsync.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Widgets/ModularUIExtensions.h"

static const FName InputFilterReason_Template = FName(TEXT("CreatingWidgetAsync"));

void UAsyncAction_CreateWidgetAsync::Cancel()
{
	Super::Cancel();

	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}
}

UAsyncAction_CreateWidgetAsync* UAsyncAction_CreateWidgetAsync::CreateWidgetAsync(UObject* InWorldContextObject, TSoftClassPtr<UUserWidget> InUserWidgetSoftClass, APlayerController* InOwningPlayer, bool bSuspendInputUntilComplete)
{
	if (InUserWidgetSoftClass.IsNull())
	{
		FFrame::KismetExecutionMessage(TEXT("CreateWidgetAsync was passed a null UserWidgetSoftClass"), ELogVerbosity::Error);
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(InWorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	UAsyncAction_CreateWidgetAsync* Action = NewObject<UAsyncAction_CreateWidgetAsync>();
	Action->UserWidgetSoftClass = InUserWidgetSoftClass;
	Action->OwningPlayer = InOwningPlayer;
	Action->World = World;
	Action->GameInstance = World->GetGameInstance();
	Action->bSuspendInputUntilComplete = bSuspendInputUntilComplete;
	Action->RegisterWithGameInstance(World);

	return Action;
}

void UAsyncAction_CreateWidgetAsync::Activate()
{
	SuspendInputToken = bSuspendInputUntilComplete ? UModularUIExtensions::SuspendInputForPlayer(OwningPlayer.Get(), InputFilterReason_Template) : NAME_None;

	TWeakObjectPtr<UAsyncAction_CreateWidgetAsync> LocalWeakThis(this);
	StreamingHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		UserWidgetSoftClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnWidgetLoaded),
		FStreamableManager::AsyncLoadHighPriority
	);

	// Setup a cancel delegate so that we can resume input if this handler is canceled.
	StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this,
	                                                                          [this]()
	                                                                          {
		                                                                          UModularUIExtensions::ResumeInputForPlayer(OwningPlayer.Get(), SuspendInputToken);
	                                                                          })
	);
}

void UAsyncAction_CreateWidgetAsync::OnWidgetLoaded()
{
	if (bSuspendInputUntilComplete)
	{
		UModularUIExtensions::ResumeInputForPlayer(OwningPlayer.Get(), SuspendInputToken);
	}

	// If the load as successful, create it, otherwise don't complete this.
	TSubclassOf<UUserWidget> UserWidgetClass = UserWidgetSoftClass.Get();
	if (UserWidgetClass)
	{
		UUserWidget* UserWidget = UWidgetBlueprintLibrary::Create(World.Get(), UserWidgetClass, OwningPlayer.Get());
		OnComplete.Broadcast(UserWidget);
	}

	StreamingHandle.Reset();

	SetReadyToDestroy();
}
