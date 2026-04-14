// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FHelperFunctionModule"

class FHelperFunctionModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FHelperFunctionModule::StartupModule()
{
}

void FHelperFunctionModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHelperFunctionModule, HelperFunction)

/*
 * 融合了Lyra中AsyncMixin插件的模块
 * 还添加了其他在项目中使用的方便函数
 */