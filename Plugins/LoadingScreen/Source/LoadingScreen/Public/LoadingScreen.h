// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/*
 * 此模块为LoadingScreen模块
 * 其中实现了加载界面
 * 覆盖了自动和手动加载界面
 * 其中自动加载界面包括了大部分的通用评估条件,其评估函数在CheckForAnyNeedToShowLoadingScreen()和其相关的评估函数
 * 其中手动加载界面则需要用户手动调用显示和隐藏加载界面,评估函数在ShouldShowLoadingScreen()和其相关的评估函数
 * 手动加载实现了蓝图调用的蓝图节点,还实现了能在GS,PC等主要类里面使用的接口类 ILoadingQueryInterface ,用于提供评估条件给加载界面
 * 总的来说加载界面模块实现了自动和手动,还可以拓展自定义的评估条件
 */
class FLoadingScreenModule : public IModuleInterface
{
public:
	/** IModuleInterface 实现 */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
