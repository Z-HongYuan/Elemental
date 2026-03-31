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
