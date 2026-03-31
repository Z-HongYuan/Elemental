// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularLoadingScreenManager.h"

#include "ModularLoadingQueryInterface.h"
#include "ModularLoadingScreenSettings.h"
#include "PreLoadScreenManager.h"
#include "ShaderPipelineCache.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/ThreadHeartBeat.h"
#include "Widgets/Images/SThrobber.h"

// 加载屏幕的分析类别
CSV_DEFINE_CATEGORY(ModularLoadingScreen, true);

DECLARE_LOG_CATEGORY_EXTERN(LogModularLoadingScreen, Log, All); //注册日志分类
DEFINE_LOG_CATEGORY(LogModularLoadingScreen); //注册日志分类

void UModularLoadingScreenManager::Initialize(FSubsystemCollectionBase& Collection)
{
	//注册函数到加载地图的监听中
	FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &ThisClass::HandlePreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);

	const UGameInstance* LocalGameInstance = GetGameInstance();
	check(LocalGameInstance);
}

void UModularLoadingScreenManager::Deinitialize()
{
	//恢复输入并且移除控件
	StopBlockingInput();
	RemoveWidgetFromViewport();

	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	// 关闭Tick
	SetTickableTickType(ETickableTickType::Never);
}

bool UModularLoadingScreenManager::ShouldCreateSubsystem(UObject* Outer) const
{
	// 只允许客户端
	const UGameInstance* GameInstance = CastChecked<UGameInstance>(Outer);
	const bool bIsServerWorld = GameInstance->IsDedicatedServerInstance();
	return !bIsServerWorld;
}

void UModularLoadingScreenManager::Tick(float DeltaTime)
{
	UpdateLoadingScreen();

	TimeUntilNextLogHeartbeatSeconds = FMath::Max(TimeUntilNextLogHeartbeatSeconds - DeltaTime, 0.0);
}

ETickableTickType UModularLoadingScreenManager::GetTickableTickType() const
{
	// 模板不执行
	if (IsTemplate()) return ETickableTickType::Never;

	return ETickableTickType::Conditional;
}

bool UModularLoadingScreenManager::IsTickable() const
{
	// 排除了没有游戏视口的情况
	UGameInstance* GameInstance = GetGameInstance();
	return (GameInstance && GameInstance->GetGameViewportClient());
}

TStatId UModularLoadingScreenManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UModularLoadingScreenManager, STATGROUP_Tickables);
}

UWorld* UModularLoadingScreenManager::GetTickableGameObjectWorld() const
{
	return GetGameInstance()->GetWorld();
}

void UModularLoadingScreenManager::RegisterLoadingProcessor(TScriptInterface<IModularLoadingQueryInterface> Interface)
{
	ExternalLoadingProcessors.Add(Interface.GetObject());
}

void UModularLoadingScreenManager::UnregisterLoadingProcessor(TScriptInterface<IModularLoadingQueryInterface> Interface)
{
	ExternalLoadingProcessors.Remove(Interface.GetObject());
}

void UModularLoadingScreenManager::HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName)
{
	if (WorldContext.OwningGameInstance == GetGameInstance())
	{
		bCurrentlyInLoadMap = true;

		// 引擎准备就绪后,马上评估一次是否显示
		if (GEngine->IsInitialized())
			UpdateLoadingScreen();
	}
}

void UModularLoadingScreenManager::HandlePostLoadMap(UWorld* World)
{
	if ((World != nullptr) && (World->GetGameInstance() == GetGameInstance()))
		bCurrentlyInLoadMap = false;
}

void UModularLoadingScreenManager::UpdateLoadingScreen()
{
	bool bLogLoadingScreenStatus = LoadingScreenCVars::LogLoadingScreenReasonEveryFrame;

	if (ShouldShowLoadingScreen())
	{
		const UModularLoadingScreenSettings* Settings = GetDefault<UModularLoadingScreenSettings>();

		// 如果我们没有在给定时间内到达指定的检查点，将触发挂起检测器，以便我们更好地确定进度停滞的位置。性能检测
		FThreadHeartBeat::Get().MonitorCheckpointStart(GetFName(), Settings->LoadingScreenHeartbeatHangDuration);

		ShowLoadingScreen();

		if ((Settings->LogLoadingScreenHeartbeatInterval > 0.0f) && (TimeUntilNextLogHeartbeatSeconds <= 0.0))
		{
			bLogLoadingScreenStatus = true;
			TimeUntilNextLogHeartbeatSeconds = Settings->LogLoadingScreenHeartbeatInterval;
		}
	}
	else
	{
		HideLoadingScreen();

		FThreadHeartBeat::Get().MonitorCheckpointEnd(GetFName());
	}

	if (bLogLoadingScreenStatus)
		UE_LOG(LogModularLoadingScreen, Log, TEXT("Loading screen showing: %d. Reason: %s"), bCurrentlyShowingLoadingScreen ? 1 : 0, *DebugReasonForShowingOrHidingLoadingScreen);
}

bool UModularLoadingScreenManager::CheckForAnyNeedToShowLoadingScreen()
{
	// 重置原因,方便溯源
	DebugReasonForShowingOrHidingLoadingScreen = TEXT("Reason for Showing/Hiding LoadingScreen is unknown!");

	const UGameInstance* LocalGameInstance = GetGameInstance();

	// 是否强制显示加载屏幕,控制台命令
	if (LoadingScreenCVars::ForceLoadingScreenVisible)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("ModularLoadingScreen.AlwaysShow is true"));
		return true;
	}

	// 如果没有世界上下文,则显示加载屏幕
	const FWorldContext* Context = LocalGameInstance->GetWorldContext();
	if (Context == nullptr)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("The game instance has a null WorldContext"));
		return true;
	}

	//如果世界无效,则显示加载屏幕
	UWorld* World = Context->World();
	if (World == nullptr)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We have no world (FWorldContext's World() is null)"));
		return true;
	}

	//游戏状态尚未复制。 游戏状态还没有准备好
	AGameStateBase* GameState = World->GetGameState<AGameStateBase>();
	if (GameState == nullptr)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("GameState hasn't yet replicated (it's null)"));
		return true;
	}

	// 正在加载地图, 显示加载屏幕
	if (bCurrentlyInLoadMap)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("bCurrentlyInLoadMap is true"));
		return true;
	}

	// 在网络中,正在等待联机Travel
	if (!Context->TravelURL.IsEmpty())
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We have pending travel (the TravelURL is not empty)"));
		return true;
	}

	// 正在连接其他服务器
	if (Context->PendingNetGame != nullptr)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We are connecting to another server (PendingNetGame != nullptr)"));
		return true;
	}

	// 游戏未开始
	if (!World->HasBegunPlay())
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("World hasn't begun play"));
		return true;
	}

	//在无缝旅行期间显示加载屏幕
	if (World->IsInSeamlessTravel())
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We are in seamless travel"));
		return true;
	}

	// 询问游戏状态是否需要加载屏幕
	if (IModularLoadingQueryInterface::ShouldShowLoadingScreen(GameState, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
		return true;


	// 询问游戏状态包含的组件是否需要加载屏幕
	for (UActorComponent* TestComponent : GameState->GetComponents())
	{
		if (IModularLoadingQueryInterface::ShouldShowLoadingScreen(TestComponent, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
		{
			return true;
		}
	}

	// 评估注册的处理器中是否需要加载屏幕
	for (const TWeakInterfacePtr<IModularLoadingQueryInterface>& Processor : ExternalLoadingProcessors)
	{
		if (IModularLoadingQueryInterface::ShouldShowLoadingScreen(Processor.GetObject(), /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
			return true;
	}

	// 检查每个本地玩家是否需要加载屏幕
	bool bFoundAnyLocalPC = false;
	bool bMissingAnyLocalPC = false;

	for (ULocalPlayer* LP : LocalGameInstance->GetLocalPlayers())
	{
		if (LP != nullptr)
		{
			if (APlayerController* PC = LP->PlayerController)
			{
				bFoundAnyLocalPC = true;

				// 询问PC本身是否需要加载屏幕
				if (IModularLoadingQueryInterface::ShouldShowLoadingScreen(PC, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
					return true;

				// 询问任何PC组件是否需要加载屏幕
				for (UActorComponent* TestComponent : PC->GetComponents())
				{
					if (IModularLoadingQueryInterface::ShouldShowLoadingScreen(TestComponent, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
						return true;
				}
			}
			else
			{
				bMissingAnyLocalPC = true;
			}
		}
	}

	UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient();
	const bool bIsInSplitscreen = GameViewportClient->GetCurrentSplitscreenConfiguration() != ESplitScreenType::None;

	// 在分屏中，评估是否所有PC都有效
	if (bIsInSplitscreen && bMissingAnyLocalPC)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("At least one missing local player controller in splitscreen"));
		return true;
	}

	// 在非分屏中，至少一个有效PC
	if (!bIsInSplitscreen && !bFoundAnyLocalPC)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("Need at least one local player controller"));
		return true;
	}

	// 完成评估,没有需要加载界面
	DebugReasonForShowingOrHidingLoadingScreen = TEXT("(nothing wants to show it anymore)");
	return false;
}

bool UModularLoadingScreenManager::ShouldShowLoadingScreen()
{
	const UModularLoadingScreenSettings* Settings = GetDefault<UModularLoadingScreenSettings>();

	// 检查强制状态的调试命令
#if !UE_BUILD_SHIPPING
	static bool bCmdLineNoLoadingScreen = FParse::Param(FCommandLine::Get(), TEXT("NoLoadingScreen"));
	if (bCmdLineNoLoadingScreen)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("CommandLine has 'NoLoadingScreen'"));
		return false;
	}
#endif

	// 如果没有游戏视口，则无法显示加载屏幕
	UGameInstance* LocalGameInstance = GetGameInstance();
	if (LocalGameInstance->GetGameViewportClient() == nullptr)
		return false;

	//检查是否需要显示加载屏幕
	const bool bNeedToShowLoadingScreen = CheckForAnyNeedToShowLoadingScreen();

	// 如果需要，请将加载屏幕保持更长时间
	bool bWantToForceShowLoadingScreen = false;
	if (bNeedToShowLoadingScreen)
	{
		// 仍需展示
		TimeLoadingScreenLastDismissed = -1.0;
	}
	else
	{
		// 不再*需要*显示屏幕，但可能仍想显示一段时间
		const double CurrentTime = FPlatformTime::Seconds();
		const bool bCanHoldLoadingScreen = (!GIsEditor || Settings->HoldLoadingScreenAdditionalSecsEvenInEditor);
		const double HoldLoadingScreenAdditionalSecs = bCanHoldLoadingScreen ? LoadingScreenCVars::HoldLoadingScreenAdditionalSecs : 0.0;

		if (TimeLoadingScreenLastDismissed < 0.0) TimeLoadingScreenLastDismissed = CurrentTime;

		const double TimeSinceScreenDismissed = CurrentTime - TimeLoadingScreenLastDismissed;

		// 再保持X秒，方便纹理流送之类的
		if ((HoldLoadingScreenAdditionalSecs > 0.0) && (TimeSinceScreenDismissed < HoldLoadingScreenAdditionalSecs))
		{
			//确保我们此时正在渲染世界，这样纹理才能真正流入
			//@TODO:如果bNeedToShowLoadingScreen在此窗口期间恢复为true，我们将不再关闭此功能。..
			UGameViewportClient* GameViewportClient = GetGameInstance()->GetGameViewportClient();
			GameViewportClient->bDisableWorldRendering = false;

			DebugReasonForShowingOrHidingLoadingScreen = FString::Printf(TEXT("Keeping loading screen up for an additional %.2f seconds to allow texture streaming"), HoldLoadingScreenAdditionalSecs);
			bWantToForceShowLoadingScreen = true;
		}
	}

	return bNeedToShowLoadingScreen || bWantToForceShowLoadingScreen;
}

bool UModularLoadingScreenManager::IsShowingInitialLoadingScreen() const
{
	FPreLoadScreenManager* PreLoadScreenManager = FPreLoadScreenManager::Get();
	return (PreLoadScreenManager != nullptr) && PreLoadScreenManager->HasValidActivePreLoadScreen();
}

void UModularLoadingScreenManager::ShowLoadingScreen()
{
	// 之前显示过就不需要了
	if (bCurrentlyShowingLoadingScreen) return;

	// 如果引擎仍在加载，则无法显示加载屏幕。判断是否还有加载任务
	if (FPreLoadScreenManager::Get() && FPreLoadScreenManager::Get()->HasActivePreLoadScreenType(EPreLoadScreenTypes::EngineLoadingScreen)) return;

	TimeLoadingScreenShown = FPlatformTime::Seconds();

	bCurrentlyShowingLoadingScreen = true;

	CSV_EVENT(ModularLoadingScreen, TEXT("Show"));

	const UModularLoadingScreenSettings* Settings = GetDefault<UModularLoadingScreenSettings>();

	if (IsShowingInitialLoadingScreen())
	{
		UE_LOG(LogModularLoadingScreen, Log, TEXT("Showing loading screen when 'IsShowingInitialLoadingScreen()' is true."));
		UE_LOG(LogModularLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);
	}
	else
	{
		UE_LOG(LogModularLoadingScreen, Log, TEXT("Showing loading screen when 'IsShowingInitialLoadingScreen()' is false."));
		UE_LOG(LogModularLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);

		UGameInstance* LocalGameInstance = GetGameInstance();

		// 阻挡输入
		StartBlockingInput();

		//广播变化性
		LoadingScreenVisibilityChanged.Broadcast(/*bIsVisible=*/ true);

		// 创建加载屏幕小部件
		TSubclassOf<UUserWidget> LoadingScreenWidgetClass = Settings->LoadingScreenWidget.TryLoadClass<UUserWidget>();
		if (UUserWidget* UserWidget = UUserWidget::CreateWidgetInstance(*LocalGameInstance, LoadingScreenWidgetClass, NAME_None))
		{
			LoadingScreenWidget = UserWidget->TakeWidget();
		}
		else
		{
			UE_LOG(LogModularLoadingScreen, Error, TEXT("Failed to load the loading screen widget %s, falling back to placeholder."), *Settings->LoadingScreenWidget.ToString());
			LoadingScreenWidget = SNew(SThrobber);
		}

		// 以高ZOrder添加到视口，以确保它覆盖大多数屏幕
		UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient();
		GameViewportClient->AddViewportWidgetContent(LoadingScreenWidget.ToSharedRef(), Settings->LoadingScreenZOrder);

		//更改性能设置
		ChangePerformanceSettings(/*bEnableLoadingScreen=*/ true);

		// 启动Tick,确保马上显示
		if (!GIsEditor || Settings->ForceTickLoadingScreenEvenInEditor)
			FSlateApplication::Get().Tick();
	}
}

void UModularLoadingScreenManager::HideLoadingScreen()
{
	if (!bCurrentlyShowingLoadingScreen) return;

	StopBlockingInput();

	if (IsShowingInitialLoadingScreen())
	{
		UE_LOG(LogModularLoadingScreen, Log, TEXT("Hiding loading screen when 'IsShowingInitialLoadingScreen()' is true."));
		UE_LOG(LogModularLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);
	}
	else
	{
		UE_LOG(LogModularLoadingScreen, Log, TEXT("Hiding loading screen when 'IsShowingInitialLoadingScreen()' is false."));
		UE_LOG(LogModularLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);

		UE_LOG(LogModularLoadingScreen, Log, TEXT("Garbage Collecting before dropping load screen"));
		GEngine->ForceGarbageCollection(true);

		// 移除加载屏幕
		RemoveWidgetFromViewport();

		// 更改性能设置
		ChangePerformanceSettings(/*bEnableLoadingScreen=*/ false);

		// 广播变化性
		LoadingScreenVisibilityChanged.Broadcast(/*bIsVisible=*/ false);
	}

	CSV_EVENT(ModularLoadingScreen, TEXT("Hide"));

	const double LoadingScreenDuration = FPlatformTime::Seconds() - TimeLoadingScreenShown;
	UE_LOG(LogModularLoadingScreen, Log, TEXT("LoadingScreen was visible for %.2fs"), LoadingScreenDuration);

	bCurrentlyShowingLoadingScreen = false;
}

void UModularLoadingScreenManager::RemoveWidgetFromViewport()
{
	UGameInstance* LocalGameInstance = GetGameInstance();
	if (LoadingScreenWidget.IsValid())
	{
		//如果视口有效,使用引用来删除对应的控件
		if (UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient())
			GameViewportClient->RemoveViewportWidgetContent(LoadingScreenWidget.ToSharedRef());

		LoadingScreenWidget.Reset();
	}
}

void UModularLoadingScreenManager::StartBlockingInput()
{
	//注册一个输入接收器,就可以拦截所有的输入
	if (!InputPreProcessor.IsValid())
	{
		InputPreProcessor = MakeShareable<FLoadingScreenInputPreProcessor>(new FLoadingScreenInputPreProcessor());
		FSlateApplication::Get().RegisterInputPreProcessor(InputPreProcessor, 0);
	}
}

void UModularLoadingScreenManager::StopBlockingInput()
{
	//移除输入接收器
	if (InputPreProcessor.IsValid())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputPreProcessor);
		InputPreProcessor.Reset();
	}
}

void UModularLoadingScreenManager::ChangePerformanceSettings(bool bEnableLoadingScreen)
{
	UGameInstance* LocalGameInstance = GetGameInstance();
	UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient();

	FShaderPipelineCache::SetBatchMode(bEnableLoadingScreen ? FShaderPipelineCache::BatchMode::Fast : FShaderPipelineCache::BatchMode::Background);

	// 在加载中,关闭世界渲染
	GameViewportClient->bDisableWorldRendering = bEnableLoadingScreen;

	// 根据加载屏幕是否打开,设置世界优先级
	if (UWorld* ViewportWorld = GameViewportClient->GetWorld())
		if (AWorldSettings* WorldSettings = ViewportWorld->GetWorldSettings(false, false))
			WorldSettings->bHighPriorityLoadingLocal = bEnableLoadingScreen;

	if (bEnableLoadingScreen)
	{
		// 当加载屏幕可见时，设置新的检测器超时倍数。
		double HangDurationMultiplier;
		if (!GConfig || !GConfig->GetDouble(TEXT("Core.System"), TEXT("LoadingScreenHangDurationMultiplier"), /*out*/ HangDurationMultiplier, GEngineIni))
			HangDurationMultiplier = 1.0;

		FThreadHeartBeat::Get().SetDurationMultiplier(HangDurationMultiplier);

		// 加载屏幕打开时，不要报告故障
		FGameThreadHitchHeartBeat::Get().SuspendHeartBeat();
	}
	else
	{
		// 当我们隐藏加载屏幕时，恢复挂起检测器超时
		FThreadHeartBeat::Get().SetDurationMultiplier(1.0);

		// 加载屏幕关闭后，恢复报告出现故障
		FGameThreadHitchHeartBeat::Get().ResumeHeartBeat();
	}
}
