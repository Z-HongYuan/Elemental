// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "ModularLoadingQueryInterface.h"
#include "UObject/Object.h"
#include "ModularLoadingQueryTask.generated.h"

#define UE_API MODULARLOADINGSCREEN_API

/* 用于在蓝图中使用的 Task,创建一个 Object 来控制加载屏幕的显示和隐藏*/
UCLASS(MinimalAPI, BlueprintType)
class UModularLoadingQueryTask : public UObject, public IModularLoadingQueryInterface
{
	GENERATED_BODY()

public:
	/*创建一个 Task ,用于强制显示加载屏幕,如果需要取消,请调用 Unregister()*/
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject"), Category = "LoadingScreen")
	static UE_API UModularLoadingQueryTask* CreateModularLoadingQueryTask(UObject* WorldContextObject, const FString& ShowLoadingScreenReason);

	// ~ Begin IModularLoadingQueryInterface interface
	UE_API virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	// ~ End IModularLoadingQueryInterface interface

	UFUNCTION(BlueprintCallable, Category = "LoadingScreen")
	UE_API void Unregister();

	UFUNCTION(BlueprintCallable, Category = "LoadingScreen")
	UE_API void SetShowLoadingScreenReason(const FString& InReason);

	UPROPERTY(Transient)
	FString Reason;
};

#undef UE_API
