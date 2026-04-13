// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "System/ModularUIHelperFunction.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "ModularRootLayoutWidget.generated.h"

#define UE_API MODULARUI_API

struct FStreamableHandle;
class UCommonActivatableWidget;
class UCommonActivatableWidgetContainerBase;

/*异步情况下,控件的推送状态*/
enum class EAsyncWidgetLayerState : uint8
{
	Canceled, //推送取消
	Initialize, //初始化
	AfterPush //推送后
};

/**
 * 用于整个UI系统中,最底层的RootWidget,包含一些基础布局
 * 我这里强制编码了四个控件栈,但是可以另外新加
* 游戏的主要游戏UI布局。此小部件类表示如何布局、推送和显示所有层
* 单个玩家的UI。分屏游戏中的每个玩家都将收到自己的主游戏布局。
 */
UCLASS(MinimalAPI, Abstract, meta=(DisableNativeTick))
class UModularRootLayoutWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/*这里将会是获取 UModularRootLayoutWidget 引用的静态函数*/
	static UE_API UModularRootLayoutWidget* GetModularRootLayoutWidget(const UObject* WorldContextObject);
	static UE_API UModularRootLayoutWidget* GetModularRootLayoutWidget(const APlayerController* PlayerController);
	static UE_API UModularRootLayoutWidget* GetModularRootLayoutWidget(const ULocalPlayer* LocalPlayer);

	/*这里是激活和停用的状态管理,停用的将会被折叠,意味着只有唯一激活,才拥有输入*/
	bool IsDormant() const { return bIsDormant; } //是否处于停用状态
	UE_API void SetIsDormant(bool InDormant); //设置是否处于停用状态

	/*这里将会是用于推送Widget的模版函数*/

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass)
	{
		return PushWidgetToLayerStackAsync<ActivatableWidgetT>(LayerName, bSuspendInputUntilComplete, ActivatableWidgetClass, [](EAsyncWidgetLayerState, ActivatableWidgetT*)
		{
		});
	}

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass,
	                                                          TFunction<void(EAsyncWidgetLayerState, ActivatableWidgetT*)> StateFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		static FName NAME_PushingWidgetToLayer("PushingWidgetToLayer");
		const FName SuspendInputToken = bSuspendInputUntilComplete ? UModularUIHelperFunction::SuspendInputForPlayer(GetOwningPlayer(), NAME_PushingWidgetToLayer) : NAME_None;

		FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
		TSharedPtr<FStreamableHandle> StreamingHandle = StreamableManager.RequestAsyncLoad(ActivatableWidgetClass.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
			                                                                                   [this, LayerName, ActivatableWidgetClass, StateFunc, SuspendInputToken]()
			                                                                                   {
				                                                                                   UModularUIHelperFunction::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);

				                                                                                   ActivatableWidgetT* Widget = PushWidgetToLayerStack<ActivatableWidgetT>(
					                                                                                   LayerName, ActivatableWidgetClass.Get(), [StateFunc](ActivatableWidgetT& WidgetToInit)
					                                                                                   {
						                                                                                   StateFunc(EAsyncWidgetLayerState::Initialize, &WidgetToInit);
					                                                                                   });

				                                                                                   StateFunc(EAsyncWidgetLayerState::AfterPush, Widget);
			                                                                                   })
		);

		// Setup a cancel delegate so that we can resume input if this handler is canceled.
		StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this,
		                                                                          [this, StateFunc, SuspendInputToken]()
		                                                                          {
			                                                                          UModularUIHelperFunction::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);
			                                                                          StateFunc(EAsyncWidgetLayerState::Canceled, nullptr);
		                                                                          })
		);

		return StreamingHandle;
	}

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass)
	{
		return PushWidgetToLayerStack<ActivatableWidgetT>(LayerName, ActivatableWidgetClass, [](ActivatableWidgetT&)
		{
		});
	}

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass, TFunctionRef<void(ActivatableWidgetT&)> InitInstanceFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		if (UCommonActivatableWidgetContainerBase* Layer = GetLayerContainer(LayerName))
		{
			return Layer->AddWidget<ActivatableWidgetT>(ActivatableWidgetClass, InitInstanceFunc);
		}

		return nullptr;
	}

	// 查找并移除 在控件容器中的控件
	UE_API void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	// 根据Tag获取对应的控件容器
	UE_API UCommonActivatableWidgetContainerBase* GetLayerContainer(FGameplayTag LayerName);

protected:
	//注册控件容器到Root控件中
	UFUNCTION(BlueprintCallable, Category="Layer")
	UE_API void RegisterLayer(UPARAM(meta = (Categories = "ModularUI.UIContainer")) FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget);

	UE_API virtual void OnIsDormantChanged();

	UE_API void OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning);

	virtual void NativeOnInitialized() override;

private:
	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetContainerBase> Container_Modal;
	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetContainerBase> Container_GameMenu;
	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetContainerBase> Container_GameHUD;
	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetContainerBase> Container_Frontend;
	UPROPERTY(Transient, meta = (Categories = "ModularUI.UIContainer"))
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Container_Layers;

	bool bIsDormant = false;

	//这样我们就能跟踪所有暂停的输入令牌，这样多个异步UI都能加载，并且在所有UI期间正确暂停。
	TArray<FName> SuspendInputTokens;
};

#undef UE_API
