// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "LoadingQueryInterface.h"
#include "UObject/Object.h"
#include "LoadingProcessTask.generated.h"

#define UE_API LOADINGSCREEN_API

/**
 * 这个是用于蓝图中的加载屏幕的任务节点
 */
UCLASS(MinimalAPI, BlueprintType)
class ULoadingProcessTask : public UObject, public ILoadingQueryInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject"))
	static UE_API ULoadingProcessTask* CreateLoadingScreenProcessTask(UObject* WorldContextObject, const FString& ShowLoadingScreenReason);

public:
	ULoadingProcessTask()
	{
	}

	UFUNCTION(BlueprintCallable)
	UE_API void Unregister();

	UFUNCTION(BlueprintCallable)
	UE_API void SetShowLoadingScreenReason(const FString& InReason);

	UE_API virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;

	FString Reason;
};

#undef UE_API
