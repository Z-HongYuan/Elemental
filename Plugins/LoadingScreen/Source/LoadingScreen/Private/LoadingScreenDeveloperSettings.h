// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "LoadingScreenDeveloperSettings.generated.h"

/*
 * 这里原本位于LoadingScreenManager的cpp文件中,但是我移动到了这里,方便分类
 * 定义了 3 个控制台变量：
 */
namespace LoadingScreenCVars
{
	// CVars
	static float HoldLoadingScreenAdditionalSecs = 2.0f;
	static FAutoConsoleVariableRef CVarHoldLoadingScreenUpAtLeastThisLongInSecs(
		TEXT("LoadingScreen.HoldLoadingScreenAdditionalSecs"),
		HoldLoadingScreenAdditionalSecs,
		TEXT("在其他加载完成后，为了让纹理流有机会避免模糊，加载屏幕应保持多长时间（以秒为单位）"),
		ECVF_Default | ECVF_Preview);

	static bool LogLoadingScreenReasonEveryFrame = false;
	static FAutoConsoleVariableRef CVarLogLoadingScreenReasonEveryFrame(
		TEXT("LoadingScreen.LogLoadingScreenReasonEveryFrame"),
		LogLoadingScreenReasonEveryFrame,
		TEXT("当此值为true时，每一帧都会将加载屏幕显示或隐藏的原因打印到日志中"),
		ECVF_Default);

	static bool ForceLoadingScreenVisible = false;
	static FAutoConsoleVariableRef CVarForceLoadingScreenVisible(
		TEXT("LoadingScreen.AlwaysShow"),
		ForceLoadingScreenVisible,
		TEXT("强制显示加载屏幕"),
		ECVF_Default);
}

/**
 * 此类是在开发者设置中定义的加载屏幕设置界面
 * 设置加载屏幕的全局设置
 */
UCLASS(Config=Game, Defaultconfig, meta=(DisplayName="加载界面开发者设置"))
class LOADINGSCREEN_API ULoadingScreenDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	ULoadingScreenDeveloperSettings();

public:
	// 加载屏幕的时候使用的控件
	UPROPERTY(Config, EditAnywhere, Category=Display, meta=(MetaClass="/Script/UMG.UserWidget"))
	FSoftClassPath LoadingScreenWidget;

	// 加载屏幕的Z轴优先级
	UPROPERTY(Config, EditAnywhere, Category=Display)
	int32 LoadingScreenZOrder = 10000;

	// 在加载完成后,控件的残余时间(/s)
	// 这里尝试给流式纹理一点缓冲时间,避免纹理模糊
	// 注意：在编辑器中，通常不会应用此设置来控制迭代时间，但可以通过 HoldLoadingScreenAdditionalSecsEvenInEditor 启用此功能
	UPROPERTY(Config, EditAnywhere, Category=Configuration, meta=(ForceUnits=s, ConsoleVariable="LoadingScreen.HoldLoadingScreenAdditionalSecs"))
	float HoldLoadingScreenAdditionalSecs = 2.0f;

	// 超过该时间（以秒为单位，非零）后，加载屏幕将被视为永久挂起
	UPROPERTY(Config, EditAnywhere, Category=Configuration, meta=(ForceUnits=s))
	float LoadingScreenHeartbeatHangDuration = 0.0f;

	// 每条记录（如果非零）之间保持加载屏幕的时间间隔（以秒为单位）。
	UPROPERTY(Config, EditAnywhere, Category=Configuration, meta=(ForceUnits=s))
	float LogLoadingScreenHeartbeatInterval = 5.0f;

	// 当该值为true时，加载屏幕显示或隐藏的原因将在每一帧打印到日志中。
	// 每帧加载屏幕日志原因
	UPROPERTY(Transient, EditAnywhere, Category=Debugging, meta=(ConsoleVariable="LoadingScreen.LogLoadingScreenReasonEveryFrame"))
	bool LogLoadingScreenReasonEveryFrame = false;

	// 强制显示加载屏幕（对调试有用）
	UPROPERTY(Transient, EditAnywhere, Category=Debugging, meta=(ConsoleVariable="LoadingScreen.AlwaysShow"))
	bool ForceLoadingScreenVisible = false;

	// 即使在编辑器中，我们也应该应用额外的HoldLoadingScreenAdditionalSecs延迟吗？（在加载屏幕开发时有用）
	// 即使在编辑器中，也保持加载屏幕额外秒数
	UPROPERTY(Transient, EditAnywhere, Category=Debugging)
	bool HoldLoadingScreenAdditionalSecsEvenInEditor = false;

	// 即使在编辑器中也要强制显示加载画面（在加载屏幕开发时有用）
	UPROPERTY(Config, EditAnywhere, Category=Configuration)
	bool ForceTickLoadingScreenEvenInEditor = true;
};
