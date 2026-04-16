// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FOwnFrameworkModule"

class FOwnFrameworkModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOwnFrameworkModule, OwnFramework)

/*
 * 这个模块重构并且集中管理了Lyra内Source的功能
 */
