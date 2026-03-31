// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_CreateWidgetAsync.generated.h"

#define UE_API MODULARGAME_API

struct FStreamableHandle;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateWidgetAsyncDelegate, UUserWidget*, UserWidget);

/**
 * 
 */
UCLASS(MinimalAPI, BlueprintType)
class UAsyncAction_CreateWidgetAsync : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UE_API virtual void Cancel() override;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"))
	static UE_API UAsyncAction_CreateWidgetAsync* CreateWidgetAsync(UObject* InWorldContextObject, TSoftClassPtr<UUserWidget> InUserWidgetSoftClass, APlayerController* InOwningPlayer, bool bSuspendInputUntilComplete = true);

	UE_API virtual void Activate() override;

public:
	UPROPERTY(BlueprintAssignable)
	FCreateWidgetAsyncDelegate OnComplete;

private:
	void OnWidgetLoaded();

	FName SuspendInputToken;
	TWeakObjectPtr<APlayerController> OwningPlayer;
	TWeakObjectPtr<UWorld> World;
	TWeakObjectPtr<UGameInstance> GameInstance;
	bool bSuspendInputUntilComplete = true;
	TSoftClassPtr<UUserWidget> UserWidgetSoftClass;
	TSharedPtr<FStreamableHandle> StreamingHandle;
};
#undef UE_API
