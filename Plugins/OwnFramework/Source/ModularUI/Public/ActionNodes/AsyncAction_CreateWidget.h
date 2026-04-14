// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_CreateWidget.generated.h"

#define  UE_API MODULARUI_API

struct FStreamableHandle;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateWidgetAsyncDelegate, UUserWidget*, UserWidget);

/* 一个创建控件的异步节点,可以取消 */
UCLASS(MinimalAPI, BlueprintType)
class UAsyncAction_CreateWidget : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/* 一个创建控件的异步节点,可以取消 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Modular UI", meta=(WorldContext = "InWorldContextObject", BlueprintInternalUseOnly="true"))
	static UE_API UAsyncAction_CreateWidget* CreateWidgetAsync(UObject* InWorldContextObject,
	                                                           TSoftClassPtr<UUserWidget> InUserWidgetSoftClass,
	                                                           APlayerController* InOwningPlayer,
	                                                           bool bSuspendInputUntilComplete = true);

	UE_API virtual void Activate() override;
	UE_API virtual void Cancel() override;

	UPROPERTY(BlueprintAssignable)
	FCreateWidgetAsyncDelegate OnCompleted;

private:
	void OnWidgetLoaded();

	FName SuspendInputToken;
	TWeakObjectPtr<APlayerController> OwningPlayer;
	TWeakObjectPtr<UWorld> World;
	TWeakObjectPtr<UGameInstance> GameInstance;
	bool bSuspendInputUntilComplete = true;
	const FName InputFilterReason_Template = FName(TEXT("CreatingWidgetAsync"));
	TSoftClassPtr<UUserWidget> UserWidgetSoftClass;
	TSharedPtr<FStreamableHandle> StreamingHandle;
};

#undef UE_API
