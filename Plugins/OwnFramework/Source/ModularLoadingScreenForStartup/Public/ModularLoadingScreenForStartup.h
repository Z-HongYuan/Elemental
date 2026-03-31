#pragma once

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FModularLoadingScreenForStartupModule"

class FModularLoadingScreenForStartupModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};


inline void FModularLoadingScreenForStartupModule::StartupModule()
{
}

inline void FModularLoadingScreenForStartupModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularLoadingScreenForStartupModule, ModularLoadingScreenForStartup)
