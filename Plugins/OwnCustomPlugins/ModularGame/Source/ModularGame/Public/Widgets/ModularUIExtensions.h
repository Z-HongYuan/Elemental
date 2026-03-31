// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonInputTypeEnum.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ModularUIExtensions.generated.h"

#define UE_API MODULARGAME_API

struct FGameplayTag;
class UCommonActivatableWidget;
/**
 * 这里是为了使用模块化UI添加的方便函数类
 */
UCLASS(MinimalAPI)
class UModularUIExtensions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UModularUIExtensions()
	{
	}

	//从控件上下文中获取玩家的输入设备类型
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Global UI Extensions", meta = (WorldContext = "WidgetContextObject"))
	static UE_API ECommonInputType GetOwningPlayerInputType(const UUserWidget* WidgetContextObject);

	//从控件上下文中获取玩家是否正在使用触摸
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Global UI Extensions", meta = (WorldContext = "WidgetContextObject"))
	static UE_API bool IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject);

	//从控件上下文中获取玩家是否正在使用游戏手柄
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Global UI Extensions", meta = (WorldContext = "WidgetContextObject"))
	static UE_API bool IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject);

	//添加内容到指定玩家层,使用同步添加
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	static UE_API UCommonActivatableWidget* PushContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer,
	                                                                     UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerName,
	                                                                     UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> WidgetClass);
	//添加内容到指定玩家层,使用异步添加
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	static UE_API void PushStreamedContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer,
	                                                        UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerName,
	                                                        UPARAM(meta = (AllowAbstract = false)) TSoftClassPtr<UCommonActivatableWidget> WidgetClass);

	//弹出指定的控件内容
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	static UE_API void PopContentFromLayer(UCommonActivatableWidget* ActivatableWidget);

	//获取玩家对应的LocalPlayer
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	static UE_API ULocalPlayer* GetLocalPlayerFromController(APlayerController* PlayerController);

	//暂停玩家输入
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	static UE_API FName SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason);

	static UE_API FName SuspendInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendReason);

	//恢复玩家输入
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Extensions")
	static UE_API void ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken);

	static UE_API void ResumeInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendToken);

private:
	static UE_API int32 InputSuspensions;
};
#undef UE_API
