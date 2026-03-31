// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "ModularLoadingScreenSettings.generated.h"

#define UE_API MODULARLOADINGSCREEN_API

/*用于保存 ModularLoadingScreen 模块的各种设置*/
UCLASS(Config=Game, Defaultconfig, meta=(DisplayName="Modular Loading Screen"))
class UModularLoadingScreenSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UModularLoadingScreenSettings() { CategoryName = TEXT("Game"); }

	// 加载屏幕显示的控件引用
	UPROPERTY(config, EditAnywhere, Category=Display, meta=(MetaClass="/Script/UMG.UserWidget"))
	FSoftClassPath LoadingScreenWidget;

	// 视口堆栈中加载屏幕控件的 z 顺序
	UPROPERTY(config, EditAnywhere, Category=Display)
	int32 LoadingScreenZOrder = 10000;

	//在其他加载完成后，将加载屏幕保持多长时间（以秒为单位） 尝试给纹理流一个避免模糊的机会
	//注意：这通常不会在编辑器中应用于迭代时间，但可以通过 HoldLoadingScreenAdditional SecsEvenInEditor 启用
	UPROPERTY(config, EditAnywhere, Category=Configuration, meta=(ForceUnits=s, ConsoleVariable="ModularLoadingScreen.HoldLoadingScreenAdditionalSecs"))
	float HoldLoadingScreenAdditionalSecs = 2.0f;

	// 加载屏幕被视为故障的持续时间 会导致一直显示加载屏幕
	UPROPERTY(config, EditAnywhere, Category=Configuration, meta=(ForceUnits=s))
	float LoadingScreenHeartbeatHangDuration = 0.0f;

	// 保持加载屏幕打开的每个日志之间的间隔（秒）（如果非零）。
	UPROPERTY(config, EditAnywhere, Category=Configuration, meta=(ForceUnits=s))
	float LogLoadingScreenHeartbeatInterval = 5.0f;

	// 是否每帧打印显示/隐藏的原因到日志中
	UPROPERTY(Transient, EditAnywhere, Category=Debugging, meta=(ConsoleVariable="ModularLoadingScreen.LogLoadingScreenReasonEveryFrame"))
	bool LogLoadingScreenReasonEveryFrame = 0;

	// 强制显示加载屏幕（调试用）
	UPROPERTY(Transient, EditAnywhere, Category=Debugging, meta=(ConsoleVariable="ModularLoadingScreen.AlwaysShow"))
	bool ForceLoadingScreenVisible = false;

	//我们是否应该在编辑器中应用额外的 HoldLoadingScreenAdditional Secs 延迟 (开发用)
	UPROPERTY(Transient, EditAnywhere, Category=Debugging)
	bool HoldLoadingScreenAdditionalSecsEvenInEditor = false;

	//我们是否应该在编辑器中应用额外的 HoldLoadingScreenAdditional Secs 延迟 (开发用)
	UPROPERTY(config, EditAnywhere, Category=Configuration)
	bool ForceTickLoadingScreenEvenInEditor = true;
};

#undef UE_API
