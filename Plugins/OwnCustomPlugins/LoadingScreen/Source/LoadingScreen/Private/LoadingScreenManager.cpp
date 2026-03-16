// Copyright © 2026 鸿源z. All Rights Reserved.


#include "LoadingScreenManager.h"

#include "LoadingQueryInterface.h"
#include "LoadingScreenDeveloperSettings.h"
#include "PreLoadScreenManager.h"
#include "ShaderPipelineCache.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/ThreadHeartBeat.h"
#include "Widgets/Images/SThrobber.h"


//@TODO: 为什么GetLocalPlayers()函数中会有空指针（nullptr）条目？这真的可能吗？
//@TODO: 在PIE模式设置为模拟的情况下进行测试，并决定加载屏幕动作应发生的频率（如果有的话）
//@TODO: 允许除GameState/PlayerController（及其拥有的组件）之外的实现ILoadingProcessInterface的其他对象注册为相关方
//@TODO: 更改音乐设置（可以在此处进行，也可以使用 LoadingScreenVisibilityChanged 委托进行）
//@TODO: 工作室分析（FireEvent_PIEFinishedLoading / 跟踪PIE启动时间以进行回归测试，可以在此处或使用LoadingScreenVisibilityChanged委托进行）

//定义一个Log分类
DECLARE_LOG_CATEGORY_EXTERN(LogLoadingScreen, Log, All);

//定义一个Log分类
DEFINE_LOG_CATEGORY(LogLoadingScreen);

// 加载界面的分析分类
CSV_DEFINE_CATEGORY(LoadingScreen, true);


void ULoadingScreenManager::Initialize(FSubsystemCollectionBase& Collection)
{
	//添加委托,在预加载地图和加载地图完成后调用
	FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &ThisClass::HandlePreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);

	//确保GameInstance有效
	const UGameInstance* LocalGameInstance = GetGameInstance();
	check(LocalGameInstance);
}

void ULoadingScreenManager::Deinitialize()
{
	//恢复接收输入
	StopBlockingInput();

	//移除加载屏幕小部件
	RemoveWidgetFromViewport();

	//移除委托
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	//关闭Tick输入
	SetTickableTickType(ETickableTickType::Never);
}

bool ULoadingScreenManager::ShouldCreateSubsystem(UObject* Outer) const
{
	// 确保只有客户端拥有加载屏幕
	const UGameInstance* GameInstance = CastChecked<UGameInstance>(Outer);
	const bool bIsServerWorld = GameInstance->IsDedicatedServerInstance();
	return !bIsServerWorld;
}

void ULoadingScreenManager::Tick(float DeltaTime)
{
	//每帧都检查评估是否显示加载屏幕
	UpdateLoadingScreen();

	//更新时间
	TimeUntilNextLogHeartbeatSeconds = FMath::Max(TimeUntilNextLogHeartbeatSeconds - DeltaTime, 0.0);
}

ETickableTickType ULoadingScreenManager::GetTickableTickType() const
{
	//检查是否为模版对象, 如果为模版对象则不执行任何操作
	if (IsTemplate())
	{
		return ETickableTickType::Never;
	}
	//有条件的Tick
	return ETickableTickType::Conditional;
}

bool ULoadingScreenManager::IsTickable() const
{
	// 如果我们没有游戏视口客户端，就不用Tick，这样可以捕捉到ShouldCreateSubsystem覆盖不了的额外情况
	UGameInstance* GameInstance = GetGameInstance();
	return (GameInstance && GameInstance->GetGameViewportClient());
}

TStatId ULoadingScreenManager::GetStatId() const
{
	//用于性能统计
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULoadingScreenManager, STATGROUP_Tickables);
}

UWorld* ULoadingScreenManager::GetTickableGameObjectWorld() const
{
	//返回当前实例的世界
	return GetGameInstance()->GetWorld();
}

void ULoadingScreenManager::RegisterLoadingProcessor(TScriptInterface<ILoadingQueryInterface> Interface)
{
	ExternalLoadingProcessors.Add(Interface.GetObject());
}

void ULoadingScreenManager::UnregisterLoadingProcessor(TScriptInterface<ILoadingQueryInterface> Interface)
{
	ExternalLoadingProcessors.Remove(Interface.GetObject());
}

void ULoadingScreenManager::HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName)
{
	if (WorldContext.OwningGameInstance == GetGameInstance())
	{
		bCurrentlyInLoadMap = true;

		// 如果引擎已初始化，则立即更新加载屏幕
		if (GEngine->IsInitialized())
		{
			UpdateLoadingScreen();
		}
	}
}

void ULoadingScreenManager::HandlePostLoadMap(UWorld* World)
{
	if ((World != nullptr) && (World->GetGameInstance() == GetGameInstance()))
	{
		bCurrentlyInLoadMap = false;
	}
}

void ULoadingScreenManager::UpdateLoadingScreen()
{
	//从控制台变量判断是否每帧日志输出
	bool bLogLoadingScreenStatus = LoadingScreenCVars::LogLoadingScreenReasonEveryFrame;

	if (ShouldShowLoadingScreen())
	{
		const ULoadingScreenDeveloperSettings* Settings = GetDefault<ULoadingScreenDeveloperSettings>();

		//如果我们未能在规定时间内到达指定检查点，就会触发挂起检测器，这样我们就能更好地确定进度停滞在何处。检测加载是否卡死
		FThreadHeartBeat::Get().MonitorCheckpointStart(GetFName(), Settings->LoadingScreenHeartbeatHangDuration);

		ShowLoadingScreen();

		//每隔固定时间（如 5 秒）强制输出一次日志 即使没有显式请求调试，也能定期看到加载进度
		if ((Settings->LogLoadingScreenHeartbeatInterval > 0.0f) && (TimeUntilNextLogHeartbeatSeconds <= 0.0))
		{
			bLogLoadingScreenStatus = true;
			TimeUntilNextLogHeartbeatSeconds = Settings->LogLoadingScreenHeartbeatInterval;
		}
	}
	else
	{
		//没有任何对象需要加载屏幕的话就隐藏它
		HideLoadingScreen();

		//结束检测
		FThreadHeartBeat::Get().MonitorCheckpointEnd(GetFName());
	}

	if (bLogLoadingScreenStatus)
	{
		UE_LOG(LogLoadingScreen, Log, TEXT("Loading screen showing: %d. Reason: %s"), bCurrentlyShowingLoadingScreen ? 1 : 0, *DebugReasonForShowingOrHidingLoadingScreen);
	}
}

bool ULoadingScreenManager::CheckForAnyNeedToShowLoadingScreen()
{
	// 在开始时，将原因填写为“未知”，以防将来有人更改此项时忘记填写原因。
	DebugReasonForShowingOrHidingLoadingScreen = TEXT("显示/隐藏加载屏幕的原因未知!");

	const UGameInstance* LocalGameInstance = GetGameInstance();

	//检测控制台变量是否强制显示加载屏幕,并输出Log
	if (LoadingScreenCVars::ForceLoadingScreenVisible)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("LoadingScreen.AlwaysShow is true"));
		return true;
	}

	//检测是否拥有世界上下文,如果没有的话,代表在切换地图之类的情况,需要显示加载屏幕
	const FWorldContext* Context = LocalGameInstance->GetWorldContext();
	if (Context == nullptr)
	{
		//我们现在没有世界背景……最好显示一个加载画面
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("The game instance has a null WorldContext"));
		return true;
	}

	//检测是否拥有世界,没有的话就需要显示加载屏幕
	UWorld* World = Context->World();
	if (World == nullptr)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We have no world (FWorldContext's World() is null)"));
		return true;
	}

	//游戏状态还没有准备就绪，我们需要显示加载屏幕
	AGameStateBase* GameState = World->GetGameState<AGameStateBase>();
	if (GameState == nullptr)
	{
		// 游戏状态尚未复制。
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("GameState hasn't yet replicated (it's null)"));
		return true;
	}

	//就是需要加载屏幕,当前在加载地图的地图
	if (bCurrentlyInLoadMap)
	{
		// 如果我们在LoadMap中，则显示加载屏幕
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("bCurrentlyInLoadMap is true"));
		return true;
	}

	// 正在加载地图,正在转换地图的时候就需要加载屏幕
	if (!Context->TravelURL.IsEmpty())
	{
		// 在转换待处理时显示加载屏幕
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We have pending travel (the TravelURL is not empty)"));
		return true;
	}

	// 当链接到一台服务器的时候需要加载屏幕
	if (Context->PendingNetGame != nullptr)
	{
		// 连接到另一台服务器
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We are connecting to another server (PendingNetGame != nullptr)"));
		return true;
	}

	//世界尚未开始游戏
	if (!World->HasBegunPlay())
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("World hasn't begun play"));
		return true;
	}

	// 在无缝旅行过程中显示加载屏幕
	if (World->IsInSeamlessTravel())
	{
		// 在无缝旅行过程中显示加载屏幕
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We are in seamless travel"));
		return true;
	}

	// 询问游戏状态是否需要加载画面
	if (ILoadingQueryInterface::ShouldShowLoadingScreen(GameState, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
	{
		return true;
	}

	// 询问任何游戏状态组件是否需要加载画面
	for (UActorComponent* TestComponent : GameState->GetComponents())
	{
		if (ILoadingQueryInterface::ShouldShowLoadingScreen(TestComponent, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
		{
			return true;
		}
	}

	//询问任何可能已注册的外部加载处理器。这些可能是由游戏代码注册的参与者或组件， 它们会指示我们在某些内容加载完成时， 保持加载屏幕的显示。
	//询问外部加载处理器,当前是否需要加载屏幕
	for (const TWeakInterfacePtr<ILoadingQueryInterface>& Processor : ExternalLoadingProcessors)
	{
		if (ILoadingQueryInterface::ShouldShowLoadingScreen(Processor.GetObject(), /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
		{
			return true;
		}
	}

	// 检查每位本地玩家是否需要加载屏幕
	bool bFoundAnyLocalPC = false;
	bool bMissingAnyLocalPC = false;

	for (ULocalPlayer* LP : LocalGameInstance->GetLocalPlayers())
	{
		if (LP != nullptr)
		{
			if (APlayerController* PC = LP->PlayerController)
			{
				bFoundAnyLocalPC = true;

				// 询问电脑本身是否需要加载屏幕
				if (ILoadingQueryInterface::ShouldShowLoadingScreen(PC, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
				{
					return true;
				}

				// 询问任何电脑组件是否需要一个加载屏幕
				for (UActorComponent* TestComponent : PC->GetComponents())
				{
					if (ILoadingQueryInterface::ShouldShowLoadingScreen(TestComponent, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
					{
						return true;
					}
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

	// 在分屏模式中，我们需要所有玩家控制器都处于启用状态
	if (bIsInSplitscreen && bMissingAnyLocalPC)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("At least one missing local player controller in splitscreen"));
		return true;
	}

	//在非分屏模式下，我们需要至少一个玩家控制器在场
	if (!bIsInSplitscreen && !bFoundAnyLocalPC)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("Need at least one local player controller"));
		return true;
	}

	// 完成！加载画面现在可以消失了,意味着以上条件都完成了,就不需要加载屏幕了
	DebugReasonForShowingOrHidingLoadingScreen = TEXT("(nothing wants to show it anymore)");
	return false;
}

bool ULoadingScreenManager::ShouldShowLoadingScreen()
{
	//获取开发者设置
	const ULoadingScreenDeveloperSettings* Settings = GetDefault<ULoadingScreenDeveloperSettings>();

	// 检查那些强制状态的调试命令
#if !UE_BUILD_SHIPPING
	static bool bCmdLineNoLoadingScreen = FParse::Param(FCommandLine::Get(), TEXT("NoLoadingScreen"));
	if (bCmdLineNoLoadingScreen)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("CommandLine has 'NoLoadingScreen'"));
		return false;
	}
#endif

	// 如果没有游戏视口，就无法显示加载画面
	UGameInstance* LocalGameInstance = GetGameInstance();
	if (LocalGameInstance->GetGameViewportClient() == nullptr)
	{
		return false;
	}

	// 检查是否需要显示加载画面
	const bool bNeedToShowLoadingScreen = CheckForAnyNeedToShowLoadingScreen();

	// 如果需要，可将加载屏幕保持更长时间
	bool bWantToForceShowLoadingScreen = false;
	if (bNeedToShowLoadingScreen)
	{
		// 仍需展示
		TimeLoadingScreenLastDismissed = -1.0;
	}
	else
	{
		// 不再“需要”展示屏幕，但可能还想展示一会儿
		const double CurrentTime = FPlatformTime::Seconds();
		const bool bCanHoldLoadingScreen = (!GIsEditor || Settings->HoldLoadingScreenAdditionalSecsEvenInEditor);
		const double HoldLoadingScreenAdditionalSecs = bCanHoldLoadingScreen ? LoadingScreenCVars::HoldLoadingScreenAdditionalSecs : 0.0;

		if (TimeLoadingScreenLastDismissed < 0.0)
		{
			TimeLoadingScreenLastDismissed = CurrentTime;
		}
		const double TimeSinceScreenDismissed = CurrentTime - TimeLoadingScreenLastDismissed;

		// 保持额外X秒，以掩盖流媒体内容
		if ((HoldLoadingScreenAdditionalSecs > 0.0) && (TimeSinceScreenDismissed < HoldLoadingScreenAdditionalSecs))
		{
			// 确保我们此时正在渲染世界，以便纹理能够真正地加载进来
			//@TODO: 如果在此窗口期间bNeedToShowLoadingScreen再次变为true，我们将不再关闭它。..
			UGameViewportClient* GameViewportClient = GetGameInstance()->GetGameViewportClient();
			GameViewportClient->bDisableWorldRendering = false;

			DebugReasonForShowingOrHidingLoadingScreen = FString::Printf(
				TEXT("Keeping loading screen up for an additional %.2f seconds to allow texture streaming"), HoldLoadingScreenAdditionalSecs);
			bWantToForceShowLoadingScreen = true;
		}
	}

	return bNeedToShowLoadingScreen || bWantToForceShowLoadingScreen;
}

bool ULoadingScreenManager::IsShowingInitialLoadingScreen() const
{
	const FPreLoadScreenManager* PreLoadScreenManager = FPreLoadScreenManager::Get();
	return (PreLoadScreenManager != nullptr) && PreLoadScreenManager->HasValidActivePreLoadScreen();
}

void ULoadingScreenManager::ShowLoadingScreen()
{
	if (bCurrentlyShowingLoadingScreen)
	{
		return;
	}

	// 如果引擎仍在加载其加载画面，则无法显示加载画面。
	if (FPreLoadScreenManager::Get() && FPreLoadScreenManager::Get()->HasActivePreLoadScreenType(EPreLoadScreenTypes::EngineLoadingScreen))
	{
		return;
	}

	TimeLoadingScreenShown = FPlatformTime::Seconds();

	bCurrentlyShowingLoadingScreen = true;

	CSV_EVENT(LoadingScreen, TEXT("Show"));

	const ULoadingScreenDeveloperSettings* Settings = GetDefault<ULoadingScreenDeveloperSettings>();

	if (IsShowingInitialLoadingScreen())
	{
		UE_LOG(LogLoadingScreen, Log, TEXT("Showing loading screen when 'IsShowingInitialLoadingScreen()' is true."));
		UE_LOG(LogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);
	}
	else
	{
		UE_LOG(LogLoadingScreen, Log, TEXT("Showing loading screen when 'IsShowingInitialLoadingScreen()' is false."));
		UE_LOG(LogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);

		UGameInstance* LocalGameInstance = GetGameInstance();

		// 在加载屏幕显示时,禁用输入
		StartBlockingInput();

		LoadingScreenVisibilityChanged.Broadcast(/*bIsVisible=*/ true);

		// 创建加载屏幕小部件
		TSubclassOf<UUserWidget> LoadingScreenWidgetClass = Settings->LoadingScreenWidget.TryLoadClass<UUserWidget>();
		if (UUserWidget* UserWidget = UUserWidget::CreateWidgetInstance(*LocalGameInstance, LoadingScreenWidgetClass, NAME_None))
		{
			LoadingScreenWidget = UserWidget->TakeWidget();
		}
		else
		{
			UE_LOG(LogLoadingScreen, Error, TEXT("Failed to load the loading screen widget %s, falling back to placeholder."), *Settings->LoadingScreenWidget.ToString());
			LoadingScreenWidget = SNew(SThrobber);
		}

		// 以较高的ZOrder添加到视口中，以确保它位于大多数内容的上方
		UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient();
		GameViewportClient->AddViewportWidgetContent(LoadingScreenWidget.ToSharedRef(), Settings->LoadingScreenZOrder);

		ChangePerformanceSettings(/*bEnableLoadingScreen=*/ true);

		if (!GIsEditor || Settings->ForceTickLoadingScreenEvenInEditor)
		{
			// 启用Slate的Tick确保加载屏幕立即显示
			FSlateApplication::Get().Tick();
		}
	}
}

void ULoadingScreenManager::HideLoadingScreen()
{
	if (!bCurrentlyShowingLoadingScreen)
	{
		return;
	}

	StopBlockingInput();

	if (IsShowingInitialLoadingScreen())
	{
		UE_LOG(LogLoadingScreen, Log, TEXT("Hiding loading screen when 'IsShowingInitialLoadingScreen()' is true."));
		UE_LOG(LogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);
	}
	else
	{
		UE_LOG(LogLoadingScreen, Log, TEXT("Hiding loading screen when 'IsShowingInitialLoadingScreen()' is false."));
		UE_LOG(LogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);

		UE_LOG(LogLoadingScreen, Log, TEXT("Garbage Collecting before dropping load screen"));
		GEngine->ForceGarbageCollection(true);

		RemoveWidgetFromViewport();

		ChangePerformanceSettings(/*bEnableLoadingScreen=*/ false);

		// 让观察者知道加载屏幕已完成
		LoadingScreenVisibilityChanged.Broadcast(/*bIsVisible=*/ false);
	}

	CSV_EVENT(LoadingScreen, TEXT("Hide"));

	const double LoadingScreenDuration = FPlatformTime::Seconds() - TimeLoadingScreenShown;
	UE_LOG(LogLoadingScreen, Log, TEXT("LoadingScreen was visible for %.2fs"), LoadingScreenDuration);

	bCurrentlyShowingLoadingScreen = false;
}

void ULoadingScreenManager::RemoveWidgetFromViewport()
{
	UGameInstance* LocalGameInstance = GetGameInstance();
	if (LoadingScreenWidget.IsValid())
	{
		if (UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient())
		{
			GameViewportClient->RemoveViewportWidgetContent(LoadingScreenWidget.ToSharedRef());
		}
		LoadingScreenWidget.Reset();
	}
}

void ULoadingScreenManager::StartBlockingInput()
{
	if (!InputPreProcessor.IsValid())
	{
		InputPreProcessor = MakeShareable<FLoadingScreenInputPreProcessor>(new FLoadingScreenInputPreProcessor());
		FSlateApplication::Get().RegisterInputPreProcessor(InputPreProcessor, 0);
	}
}

void ULoadingScreenManager::StopBlockingInput()
{
	if (InputPreProcessor.IsValid())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputPreProcessor);
		InputPreProcessor.Reset();
	}
}

void ULoadingScreenManager::ChangePerformanceSettings(bool bEnabingLoadingScreen)
{
	UGameInstance* LocalGameInstance = GetGameInstance();
	UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient();

	FShaderPipelineCache::SetBatchMode(bEnabingLoadingScreen ? FShaderPipelineCache::BatchMode::Fast : FShaderPipelineCache::BatchMode::Background);

	// 加载屏幕时禁用世界渲染
	GameViewportClient->bDisableWorldRendering = bEnabingLoadingScreen;

	// 如果加载画面出现，请确保在各个关卡中优先进行流媒体播放
	if (UWorld* ViewportWorld = GameViewportClient->GetWorld())
	{
		if (AWorldSettings* WorldSettings = ViewportWorld->GetWorldSettings(false, false))
		{
			WorldSettings->bHighPriorityLoadingLocal = bEnabingLoadingScreen;
		}
	}

	if (bEnabingLoadingScreen)
	{
		// 当加载屏幕可见时，设置新的挂起检测超时倍数。
		double HangDurationMultiplier;
		if (!GConfig || !GConfig->GetDouble(TEXT("Core.System"), TEXT("LoadingScreenHangDurationMultiplier"), /*out*/ HangDurationMultiplier, GEngineIni))
		{
			HangDurationMultiplier = 1.0;
		}
		FThreadHeartBeat::Get().SetDurationMultiplier(HangDurationMultiplier);

		// 加载屏幕显示时，请勿报告故障
		FGameThreadHitchHeartBeat::Get().SuspendHeartBeat();
	}
	else
	{
		// 当我们隐藏加载屏幕时，恢复悬挂检测超时设置
		FThreadHeartBeat::Get().SetDurationMultiplier(1.0);

		// 加载屏幕已关闭，现在恢复报告出现故障
		FGameThreadHitchHeartBeat::Get().ResumeHeartBeat();
	}
}
