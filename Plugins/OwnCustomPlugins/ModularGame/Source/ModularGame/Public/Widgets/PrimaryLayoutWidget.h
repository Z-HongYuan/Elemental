// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "ModularUIExtensions.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "PrimaryLayoutWidget.generated.h"

#define UE_API MODULARGAME_API

struct FStreamableHandle;
class UCommonActivatableWidget;
class UCommonActivatableWidgetContainerBase;
struct FGameplayTag;

/*UI的异步加载操作的状态。*/
enum class EAsyncWidgetLayerState : uint8
{
	Canceled,
	Initialize,
	AfterPush
};

/**
 * 模块化UI系统使用的主布局,其构成了UI的Root根
 * 游戏的主要游戏UI布局。此小部件类表示如何布局、推送和显示所有层
 * 单个玩家的UI。分屏游戏中的每个玩家都将收到自己的主游戏布局。
 */
UCLASS(MinimalAPI, Abstract, meta = (DisableNativeTick))
class UPrimaryLayoutWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	static UE_API UPrimaryLayoutWidget* GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject);
	static UE_API UPrimaryLayoutWidget* GetPrimaryGameLayout(APlayerController* PlayerController);
	static UE_API UPrimaryLayoutWidget* GetPrimaryGameLayout(ULocalPlayer* LocalPlayer);

	UE_API UPrimaryLayoutWidget(const FObjectInitializer& ObjectInitializer);

	/**休眠的根布局被折叠，只对拥有它的玩家注册的持久操作做出响应*/
	UE_API void SetIsDormant(bool InDormant);
	bool IsDormant() const { return bIsDormant; }


	/*异步推送,多了一步加载资源的包装*/
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass)
	{
		return PushWidgetToLayerStackAsync<ActivatableWidgetT>(LayerName, bSuspendInputUntilComplete, ActivatableWidgetClass, [](EAsyncWidgetLayerState, ActivatableWidgetT*)
		{
		});
	}

	/*异步推送,多了一步加载资源的包装*/
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass,
	                                                          TFunction<void(EAsyncWidgetLayerState, ActivatableWidgetT*)> StateFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		static FName NAME_PushingWidgetToLayer("PushingWidgetToLayer");
		const FName SuspendInputToken = bSuspendInputUntilComplete ? UModularUIExtensions::SuspendInputForPlayer(GetOwningPlayer(), NAME_PushingWidgetToLayer) : NAME_None;

		FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
		TSharedPtr<FStreamableHandle> StreamingHandle = StreamableManager.RequestAsyncLoad(ActivatableWidgetClass.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
			                                                                                   [this, LayerName, ActivatableWidgetClass, StateFunc, SuspendInputToken]()
			                                                                                   {
				                                                                                   UModularUIExtensions::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);

				                                                                                   ActivatableWidgetT* Widget = PushWidgetToLayerStack<ActivatableWidgetT>(
					                                                                                   LayerName, ActivatableWidgetClass.Get(), [StateFunc](ActivatableWidgetT& WidgetToInit)
					                                                                                   {
						                                                                                   StateFunc(EAsyncWidgetLayerState::Initialize, &WidgetToInit);
					                                                                                   });

				                                                                                   StateFunc(EAsyncWidgetLayerState::AfterPush, Widget);
			                                                                                   })
		);

		// 设置一个取消委托，以便在取消此处理程序时可以继续输入。
		StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this,
		                                                                          [this, StateFunc, SuspendInputToken]()
		                                                                          {
			                                                                          UModularUIExtensions::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);
			                                                                          StateFunc(EAsyncWidgetLayerState::Canceled, nullptr);
		                                                                          })
		);

		return StreamingHandle;
	}

	/*同步推送*/
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass)
	{
		return PushWidgetToLayerStack<ActivatableWidgetT>(LayerName, ActivatableWidgetClass, [](ActivatableWidgetT&)
		{
		});
	}

	/*同步推送*/
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass, TFunctionRef<void(ActivatableWidgetT&)> InitInstanceFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		if (UCommonActivatableWidgetContainerBase* Layer = GetLayerWidget(LayerName))
			return Layer->AddWidget<ActivatableWidgetT>(ActivatableWidgetClass, InitInstanceFunc);

		return nullptr;
	}

	// 如果小部件存在于任何层上，请找到它并将其从层中删除。
	UE_API void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	// 获取给定层标签的层小部件。
	UE_API UCommonActivatableWidgetContainerBase* GetLayerWidget(FGameplayTag LayerName);


	/** 注册一个可以推送小部件的层. */
	UFUNCTION(BlueprintCallable, Category="Layer")
	UE_API void RegisterLayer(UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget);

	UE_API virtual void OnIsDormantChanged();

	UE_API void OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning);

private:
	bool bIsDormant = false;

	// 让我们跟踪所有挂起的输入令牌，以便可以加载多个异步UI，并正确挂起
	// 在所有这些期间。
	TArray<FName> SuspendInputTokens;

	// 主布局的注册栈。
	UPROPERTY(Transient, meta = (Categories = "UI.Layer"))
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;
};

#undef UE_API
