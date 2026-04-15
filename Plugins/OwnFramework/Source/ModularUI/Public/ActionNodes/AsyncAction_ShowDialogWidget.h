// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_ShowDialogWidget.generated.h"

#define UE_API MODULARUI_API

enum class EModularMessagingResult : uint8;
class UModularGameDialogDataObj;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FModularMessagingResultBPDelegate, EModularMessagingResult, Result);

/* 用于推送对话框,并且等待玩家的选择结果 */
UCLASS(MinimalAPI)
class UAsyncAction_ShowDialogWidget : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static UAsyncAction_ShowDialogWidget* ShowConfirmationYesNo(UObject* InWorldContextObject, FText Title, FText Message);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static UAsyncAction_ShowDialogWidget* ShowConfirmationOkCancel(UObject* InWorldContextObject, FText Title, FText Message);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static UAsyncAction_ShowDialogWidget* ShowConfirmationCustom(UObject* InWorldContextObject, UModularGameDialogDataObj* DialogDataObj);

	UPROPERTY(BlueprintAssignable)
	FModularMessagingResultBPDelegate OnResulted;

	virtual void Activate() override;

private:
	void HandleConfirmationResult(EModularMessagingResult ConfirmationResult);

	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> TargetLocalPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UModularGameDialogDataObj> DialogDataObj;
};

#undef UE_API
