// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_PushContentToLayerForPlayer.generated.h"

#define UE_API MODULARGAME_API

struct FStreamableHandle;
class UCommonActivatableWidget;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPushContentToLayerForPlayerAsyncDelegate, UCommonActivatableWidget*, UserWidget);

/**
 * 
 */
UCLASS(MinimalAPI, BlueprintType)
class UAsyncAction_PushContentToLayerForPlayer : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UE_API virtual void Cancel() override;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"))
	static UE_API UAsyncAction_PushContentToLayerForPlayer* PushContentToLayerForPlayer(APlayerController* InOwningPlayer, UPARAM(meta = (AllowAbstract=false)) TSoftClassPtr<UCommonActivatableWidget> InWidgetClass,
	                                                                                    UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag InLayerName, bool bSuspendInputUntilComplete = true);

	UE_API virtual void Activate() override;

public:
	UPROPERTY(BlueprintAssignable)
	FPushContentToLayerForPlayerAsyncDelegate BeforePush;

	UPROPERTY(BlueprintAssignable)
	FPushContentToLayerForPlayerAsyncDelegate AfterPush;

private:
	FGameplayTag LayerName;
	bool bSuspendInputUntilComplete = false;
	TWeakObjectPtr<APlayerController> OwningPlayerPtr;
	TSoftClassPtr<UCommonActivatableWidget> WidgetClass;

	TSharedPtr<FStreamableHandle> StreamingHandle;
};
#undef UE_API
