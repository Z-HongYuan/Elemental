// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FModularGameSubtitlesModule"

class FModularGameSubtitlesModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FModularGameSubtitlesModule::StartupModule()
{
}

void FModularGameSubtitlesModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularGameSubtitlesModule, ModularGameSubtitles)
