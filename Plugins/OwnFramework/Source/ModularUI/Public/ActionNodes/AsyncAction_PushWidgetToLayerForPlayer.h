// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_PushWidgetToLayerForPlayer.generated.h"

#define  UE_API MODULARUI_API

struct FStreamableHandle;
class UCommonActivatableWidget;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPushWidgetToLayerForPlayerAsyncDelegate, UCommonActivatableWidget*, UserWidget);

/*异步推送软引用控件到指定Tag容器中,可取消*/
UCLASS(MinimalAPI, BlueprintType)
class UAsyncAction_PushWidgetToLayerForPlayer : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="Modular UI", meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"))
	static UE_API UAsyncAction_PushWidgetToLayerForPlayer* PushContentToLayerForPlayer(APlayerController* InOwningPlayer,
	                                                                                   UPARAM(meta = (AllowAbstract=false)) TSoftClassPtr<UCommonActivatableWidget> InWidgetClass,
	                                                                                   UPARAM(meta = (Categories = "ModularUI.UIContainer")) FGameplayTag InContainerTag,
	                                                                                   bool bSuspendInputUntilComplete = true);

	UE_API virtual void Activate() override;
	UE_API virtual void Cancel() override;

public:
	UPROPERTY(BlueprintAssignable)
	FPushWidgetToLayerForPlayerAsyncDelegate BeforePush;

	UPROPERTY(BlueprintAssignable)
	FPushWidgetToLayerForPlayerAsyncDelegate AfterPush;

private:
	FGameplayTag ContainerTag;
	bool bSuspendInputUntilComplete = false;
	TWeakObjectPtr<APlayerController> OwningPlayerPtr;
	TSoftClassPtr<UCommonActivatableWidget> WidgetClass;

	TSharedPtr<FStreamableHandle> StreamingHandle;
};

#undef UE_API
