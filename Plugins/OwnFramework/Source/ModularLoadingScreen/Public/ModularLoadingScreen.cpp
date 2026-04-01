// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FModularLoadingScreenModule"

class FModularLoadingScreenModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

inline void FModularLoadingScreenModule::StartupModule()
{
}

inline void FModularLoadingScreenModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularLoadingScreenModule, ModularLoadingScreen)

/*
 * 一个通用的加载屏幕模块,使用开发者设置参数,支持蓝图和C++
 * 使用方法:
 * 1. 使用蓝图创建一个TaskObject,用于强制显示加载屏幕
 */
