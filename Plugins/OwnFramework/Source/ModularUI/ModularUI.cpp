// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FModularUIModule"

class FModularUIModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FModularUIModule::StartupModule()
{
}

void FModularUIModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularUIModule, ModularUI)

/*
 * 针对于CommonUI的拓展
 * 融合了标签系统,实现了模块化UI系统
 */