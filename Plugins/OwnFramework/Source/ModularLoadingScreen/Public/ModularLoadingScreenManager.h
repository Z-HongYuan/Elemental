// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "Framework/Application/IInputProcessor.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ModularLoadingScreenManager.generated.h"

#define UE_API MODULARLOADINGSCREEN_API

class IModularLoadingQueryInterface;


// 用于阻止输入的输入处理器,仅在非编辑器下有效
class FLoadingScreenInputPreProcessor : public IInputProcessor
{
public:
	FLoadingScreenInputPreProcessor()
	{
	}

	virtual ~FLoadingScreenInputPreProcessor()
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
	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override { return CanEatInput(); }
	virtual bool HandleMotionDetectedEvent(FSlateApplication& SlateApp, const FMotionEvent& MotionEvent) override { return CanEatInput(); }
	//~End of IInputProcess interface
};


// 用于在编辑器内注册性能命名
namespace LoadingScreenCVars
{
	// CVars
	static float HoldLoadingScreenAdditionalSecs = 2.0f;
	static FAutoConsoleVariableRef CVarHoldLoadingScreenUpAtLeastThisLongInSecs(
		TEXT("ModularLoadingScreen.HoldLoadingScreenAdditionalSecs"),
		HoldLoadingScreenAdditionalSecs,
		TEXT("How long to hold the loading screen up after other loading finishes (in seconds) to try to give texture streaming a chance to avoid blurriness"),
		ECVF_Default | ECVF_Preview);

	static bool LogLoadingScreenReasonEveryFrame = false;
	static FAutoConsoleVariableRef CVarLogLoadingScreenReasonEveryFrame(
		TEXT("ModularLoadingScreen.LogLoadingScreenReasonEveryFrame"),
		LogLoadingScreenReasonEveryFrame,
		TEXT("When true, the reason the loading screen is shown or hidden will be printed to the log every frame."),
		ECVF_Default);

	static bool ForceLoadingScreenVisible = false;
	static FAutoConsoleVariableRef CVarForceLoadingScreenVisible(
		TEXT("ModularLoadingScreen.AlwaysShow"),
		ForceLoadingScreenVisible,
		TEXT("Force the loading screen to show."),
		ECVF_Default);
}


/**
 * 显示/隐藏加载屏幕管理器
 */
UCLASS(MinimalAPI)
class UModularLoadingScreenManager : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~End of USubsystem interface


	//~FTickableObjectBase interface
	UE_API virtual void Tick(float DeltaTime) override;
	UE_API virtual ETickableTickType GetTickableTickType() const override;
	UE_API virtual bool IsTickable() const override;
	UE_API virtual TStatId GetStatId() const override;
	UE_API virtual UWorld* GetTickableGameObjectWorld() const override;
	//~End of FTickableObjectBase interface

	//获取当前显示加载屏幕的原因
	UFUNCTION(BlueprintCallable, Category=LoadingScreen)
	FString GetDebugReasonForShowingOrHidingLoadingScreen() const { return DebugReasonForShowingOrHidingLoadingScreen; }

	//获取当前是否正在显示加载屏幕 
	bool GetLoadingScreenDisplayStatus() const { return bCurrentlyShowingLoadingScreen; }

	//加载屏幕可见性变化时调用的委托
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadingScreenVisibilityChangedDelegate, bool);
	FORCEINLINE FOnLoadingScreenVisibilityChangedDelegate& OnLoadingScreenVisibilityChangedDelegate() { return LoadingScreenVisibilityChanged; }

	UE_API void RegisterLoadingProcessor(TScriptInterface<IModularLoadingQueryInterface> Interface);
	UE_API void UnregisterLoadingProcessor(TScriptInterface<IModularLoadingQueryInterface> Interface);

private:
	UE_API void HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName);
	UE_API void HandlePostLoadMap(UWorld* World);

	//每帧评估是否显示
	UE_API void UpdateLoadingScreen();

	//评估常用的固定条件
	UE_API bool CheckForAnyNeedToShowLoadingScreen();

	//评估是否需要手动显示,包括调试命令和开发者设置之类的
	UE_API bool ShouldShowLoadingScreen();

	// 如果我们在使用此屏幕之前处于初始加载流中，则返回true
	UE_API bool IsShowingInitialLoadingScreen() const;

	// 显示加载屏幕,在视口上设置加载屏幕控件
	UE_API void ShowLoadingScreen();

	// 隐藏加载屏幕,控件将被销毁
	UE_API void HideLoadingScreen();

	// 移除并且删除控件
	UE_API void RemoveWidgetFromViewport();

	// 阻止输入
	UE_API void StartBlockingInput();

	// 恢复输入
	UE_API void StopBlockingInput();

	// 更改性能设置,减少在加载的时候的性能损失
	UE_API void ChangePerformanceSettings(bool bEnableLoadingScreen);

private:
	// 加载屏幕可见性变化时调用的委托
	FOnLoadingScreenVisibilityChangedDelegate LoadingScreenVisibilityChanged;

	// 加载屏幕控件的引用,可能为空
	TSharedPtr<SWidget> LoadingScreenWidget;

	// 阻挡玩家输入的处理器
	TSharedPtr<IInputProcessor> InputPreProcessor;

	// 注册的处理器引用,用于评估是否显示
	TArray<TWeakInterfacePtr<IModularLoadingQueryInterface>> ExternalLoadingProcessors;

	// 调试信息,未打开的原因
	FString DebugReasonForShowingOrHidingLoadingScreen;

	// 开始显示的时间
	double TimeLoadingScreenShown = 0.0;

	//加载屏幕最近想要被关闭的时间（由于最小显示持续时间要求，可能仍在运行）
	double TimeLoadingScreenLastDismissed = -1.0;

	// 直到下一个日志显示加载屏幕仍处于打开状态的时间
	double TimeUntilNextLogHeartbeatSeconds = 0.0;

	// 当前是否正在加载地图
	bool bCurrentlyInLoadMap = false;

	// 当前是否正在显示
	bool bCurrentlyShowingLoadingScreen = false;
};

#undef UE_API
