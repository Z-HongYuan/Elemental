// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FPocketWorldsModule"

class FPocketWorldsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FPocketWorldsModule::StartupModule()
{
}

void FPocketWorldsModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPocketWorldsModule, PocketWorlds)

/*
 * 1. 流送小型关卡
 * 2. 捕获场景缩略图,然后渲染到纹理中
 */
