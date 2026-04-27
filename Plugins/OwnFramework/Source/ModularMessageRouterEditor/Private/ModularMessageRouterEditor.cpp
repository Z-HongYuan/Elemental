// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FModularMessageRouterEditorModule"

class FModularMessageRouterEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FModularMessageRouterEditorModule::StartupModule()
{
}

void FModularMessageRouterEditorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularMessageRouterEditorModule, ModularMessageRouterEditor)
