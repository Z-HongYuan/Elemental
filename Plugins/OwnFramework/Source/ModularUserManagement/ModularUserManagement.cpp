// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FModularUserManagementModule"

class FModularUserManagementModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FModularUserManagementModule::StartupModule()
{
}

void FModularUserManagementModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularUserManagementModule, ModularUserManagement)

/*
 * 针对于用户的管理
 * 1. 从各种游戏平台和在线平台中获取用户信息,用户的管理
 * 2. 提供针对于会话的管理
 */