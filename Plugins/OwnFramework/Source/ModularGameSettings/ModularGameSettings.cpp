// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FModularGameSettingsModule"

class FModularGameSettingsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FModularGameSettingsModule::StartupModule()
{
}

void FModularGameSettingsModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularGameSettingsModule, ModularGameSettings)

/*
 * 用于定义游戏特定设置并将其暴露给UI的系统。
 */