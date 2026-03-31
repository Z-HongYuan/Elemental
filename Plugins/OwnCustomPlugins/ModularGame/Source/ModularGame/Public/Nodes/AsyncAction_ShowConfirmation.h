// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_ShowConfirmation.generated.h"

enum class EModularMessagingResult : uint8;
class UModularGameDialogDescriptor;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCommonMessagingResultMCDelegate, EModularMessagingResult, Result);

/**
 * 允许在蓝图中轻松触发异步确认对话框，然后您可以等待结果。
 */
UCLASS()
class UAsyncAction_ShowConfirmation : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static UAsyncAction_ShowConfirmation* ShowConfirmationYesNo(
		UObject* InWorldContextObject, FText Title, FText Message
	);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static UAsyncAction_ShowConfirmation* ShowConfirmationOkCancel(
		UObject* InWorldContextObject, FText Title, FText Message
	);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static UAsyncAction_ShowConfirmation* ShowConfirmationCustom(
		UObject* InWorldContextObject, UModularGameDialogDescriptor* Descriptor
	);

	virtual void Activate() override;

public:
	UPROPERTY(BlueprintAssignable)
	FCommonMessagingResultMCDelegate OnResult;

private:
	void HandleConfirmationResult(EModularMessagingResult ConfirmationResult);

	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> TargetLocalPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UModularGameDialogDescriptor> Descriptor;
};
