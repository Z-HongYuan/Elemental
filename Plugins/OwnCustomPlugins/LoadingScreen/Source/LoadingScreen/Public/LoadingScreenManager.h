// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Application/IInputProcessor.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LoadingScreenManager.generated.h"

#define UE_API LOADINGSCREEN_API

class ILoadingQueryInterface;


/*
 * 在显示加载画面时加入输入处理器 // 这样会捕获所有输入，因此加载画面下的活动菜单将无法进行交互
 * 这个是 FLoadingScreenInputPreProcessor
 * 预输入处理器,专门用于吞掉输入,例如：鼠标、键盘、手柄等操作
 */
class FLoadingScreenInputPreProcessor : public IInputProcessor
{
public:
	FLoadingScreenInputPreProcessor()
	{
	}

	virtual ~FLoadingScreenInputPreProcessor() override
	{
	}

	bool CanEatInput() const
	{
		return !GIsEditor;
	}

	//~IInputProcess interface
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
	{
	}

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override { return CanEatInput(); }
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override { return CanEatInput(); }
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override { return CanEatInput(); }
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }
	virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }

	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override
	{
		return CanEatInput();
	}

	virtual bool HandleMotionDetectedEvent(FSlateApplication& SlateApp, const FMotionEvent& MotionEvent) override { return CanEatInput(); }
	//~End of IInputProcess interface
};

/**
 * 用于在游戏中显示加载屏幕的控制中心
 */
UCLASS(MinimalAPI)
class ULoadingScreenManager : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~End of USubsystem interface

	//~FTickableObjectBase interface Tick的接口函数
	UE_API virtual void Tick(float DeltaTime) override;
	UE_API virtual ETickableTickType GetTickableTickType() const override;
	UE_API virtual bool IsTickable() const override;
	UE_API virtual TStatId GetStatId() const override;
	UE_API virtual UWorld* GetTickableGameObjectWorld() const override;
	//~End of FTickableObjectBase interface

	//用于获取显示或隐藏加载屏幕的调试原因
	UFUNCTION(BlueprintCallable, Category=LoadingScreen)
	FString GetDebugReasonForShowingOrHidingLoadingScreen() const
	{
		return DebugReasonForShowingOrHidingLoadingScreen;
	}

	// 当加载屏幕当前正在显示时，返回True
	bool GetLoadingScreenDisplayStatus() const
	{
		return bCurrentlyShowingLoadingScreen;
	}

	// 当加载屏幕的可见性发生变化时调用 这是一个委托,目前是C++委托,如果蓝图需要则改变一下
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadingScreenVisibilityChangedDelegate, bool);
	FORCEINLINE FOnLoadingScreenVisibilityChangedDelegate& OnLoadingScreenVisibilityChangedDelegate() { return LoadingScreenVisibilityChanged; }

	//注册和解除注册加载处理器,把处理器添加到加载处理器列表中,检查的时候会评估这个列表
	UE_API void RegisterLoadingProcessor(TScriptInterface<ILoadingQueryInterface> Interface);
	UE_API void UnregisterLoadingProcessor(TScriptInterface<ILoadingQueryInterface> Interface);

private:
	//绑定委托到加载地图前后两个钩子
	UE_API void HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName);
	UE_API void HandlePostLoadMap(UWorld* World);

	//确定显示还是隐藏加载屏幕。每帧调用
	UE_API void UpdateLoadingScreen();

	//如果我们需要显示加载屏幕，则返回true,这是一个工具函数,检查的通用的大部分状态条件
	UE_API bool CheckForAnyNeedToShowLoadingScreen();

	// 如果我们想要显示加载屏幕（无论是出于需要还是出于其他原因而人为强制显示），则返回true。,也算是一个工具函数,在Update中被调用
	UE_API bool ShouldShowLoadingScreen();

	// 如果当前处于初始加载流程中，且尚未使用此屏幕，则返回true
	UE_API bool IsShowingInitialLoadingScreen() const;

	// 显示加载屏幕。在视口上设置加载屏幕小部件
	UE_API void ShowLoadingScreen();

	// 隐藏加载屏幕。加载屏幕小部件将被销毁,将会恢复性能设置并且恢复输入
	UE_API void HideLoadingScreen();

	// 如果小部件有效,就移除视口并销毁
	UE_API void RemoveWidgetFromViewport();

	// 在加载画面可见时，禁止在游戏内使用输入,使用输入处理器的创建并注册
	UE_API void StartBlockingInput();

	// 如果当前是阻挡输入则恢复游戏内输入,使用输入处理器的销毁并卸载
	UE_API void StopBlockingInput();

	// 当加载屏幕展示的时候更改性能设置,例如避免渲染
	UE_API void ChangePerformanceSettings(bool bEnabingLoadingScreen);

private:
	// 当加载屏幕可见性发生变化时，进行广播委托
	FOnLoadingScreenVisibilityChangedDelegate LoadingScreenVisibilityChanged;

	// 正在显示的加载屏幕小部件引用（如果有的话）
	TSharedPtr<SWidget> LoadingScreenWidget;

	// 在加载屏幕显示时，输入处理器会接收所有输入
	TSharedPtr<IInputProcessor> InputPreProcessor;

	// 外部加载处理器、组件或参与者可能会延迟加载,处理器数组,用于评估是否加载屏幕
	TArray<TWeakInterfacePtr<ILoadingQueryInterface>> ExternalLoadingProcessors;

	// 加载屏幕出现（或未出现）的原因
	FString DebugReasonForShowingOrHidingLoadingScreen;

	// 我们开始显示加载屏幕的时间
	double TimeLoadingScreenShown = 0.0;

	// 加载屏幕最近一次希望被关闭的时间（由于最小显示时长要求，可能仍然处于显示状态）
	double TimeLoadingScreenLastDismissed = -1.0;

	// 距离下次记录加载屏幕为何仍然显示的时间??
	double TimeUntilNextLogHeartbeatSeconds = 0.0;

	// 当前是否在加载地图
	bool bCurrentlyInLoadMap = false;

	// 当前是否正在显示
	bool bCurrentlyShowingLoadingScreen = false;
};

#undef UE_API
