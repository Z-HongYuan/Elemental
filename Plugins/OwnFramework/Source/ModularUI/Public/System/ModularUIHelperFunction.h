// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ModularUIHelperFunction.generated.h"

#define UE_API MODULARUI_API

struct FGameplayTag;
class UCommonActivatableWidget;
enum class ECommonInputType : uint8;

/**
 * 
 */
UCLASS(MinimalAPI)
class UModularUIHelperFunction : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//获取本控件关联的玩家输入类型
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Global UI Helper Function", meta = (WorldContext = "WidgetContextObject"), DisplayName="获取拥有玩家输入类型")
	static UE_API ECommonInputType GetOwningPlayerInputType(const UUserWidget* WidgetContextObject);

	//获取本控件关联的玩家输入类型
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Global UI Helper Function", meta = (WorldContext = "WidgetContextObject"), DisplayName="拥有玩家使用触摸")
	static UE_API bool IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject);

	//获取本控件关联的玩家输入类型
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Global UI Helper Function", meta = (WorldContext = "WidgetContextObject"), DisplayName="拥有玩家使用手柄")
	static UE_API bool IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject);

	//同步方式推送指定的容器中的指定控件
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Helper Function", DisplayName="推送控件到容器内")
	static UE_API UCommonActivatableWidget* PushWidgetToLayer_ForPlayer(const ULocalPlayer* LocalPlayer,
	                                                                    UPARAM(meta = (Categories = "ModularUI.UIContainer")) FGameplayTag LayerName,
	                                                                    UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> WidgetClass);

	//异步方式推送指定的容器中的指定控件
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Helper Function", DisplayName="异步推送控件到容器内")
	static UE_API void PushWidgetToLayerAsync_ForPlayer(const ULocalPlayer* LocalPlayer,
	                                                    UPARAM(meta = (Categories = "ModularUI.UIContainer")) FGameplayTag LayerName,
	                                                    UPARAM(meta = (AllowAbstract = false)) TSoftClassPtr<UCommonActivatableWidget> WidgetClass);

	//弹出指定的容器中的指定控件
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Helper Function", DisplayName="从容器内弹出控件")
	static UE_API void PopWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	//从玩家控制器中获取本地玩家
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Helper Function", DisplayName="从玩家控制器中获取本地玩家")
	static UE_API ULocalPlayer* GetLocalPlayerFromController(APlayerController* PlayerController);

	//暂停玩家输入
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Helper Function", DisplayName="暂停玩家输入")
	static UE_API FName SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason);
	static UE_API FName SuspendInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendReason);

	//恢复玩家输入
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Global UI Helper Function", DisplayName="恢复玩家输入")
	static UE_API void ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken);
	static UE_API void ResumeInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendToken);

private:
	//暂停输入的数量
	static UE_API int32 InputSuspensions;
};

#undef UE_API
