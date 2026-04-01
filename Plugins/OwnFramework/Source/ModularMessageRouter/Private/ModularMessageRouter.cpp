// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FModularMessageRouterModule"

class FModularMessageRouterModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FModularMessageRouterModule::StartupModule()
{
}

void FModularMessageRouterModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularMessageRouterModule, ModularMessageRouter)

/*
 * 基本用法:
 * 1. 在蓝图中使用异步节点,注册监听器
 * 2. 在其他地方广播消息,并且附带对应的结构体
 * 3. 在监听的地方触发回调
 */