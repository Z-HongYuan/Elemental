// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModularPreLoadScreen.h"
#include "PreLoadScreenManager.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FModularLoadingScreenForStartupModule"

class FModularLoadingScreenForStartupModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool IsGameModule() const override;

private:
	void OnPreLoadScreenManagerCleanUp();

	/*TSharedPtr是一个非侵入性的引用计数权威对象指针。当可选的Mode模板参数设置为ThreadSafe时，此共享指针将是有条件的线程安全的。*/
	TSharedPtr<FModularPreLoadScreen> PreLoadingScreen;
};


inline void FModularLoadingScreenForStartupModule::StartupModule()
{
	//无需在专用服务器上加载这些资产。
	//仍然想在Commandlets中加载它们，以便cook捕获它们
	if (!IsRunningDedicatedServer())
	{
		PreLoadingScreen = MakeShared<FModularPreLoadScreen>();
		PreLoadingScreen->Init();

		if (!GIsEditor && FApp::CanEverRender() && FPreLoadScreenManager::Get())
		{
			//注册预加载屏幕
			FPreLoadScreenManager::Get()->RegisterPreLoadScreen(PreLoadingScreen);
			FPreLoadScreenManager::Get()->OnPreLoadScreenManagerCleanUp.AddRaw(this, &FModularLoadingScreenForStartupModule::OnPreLoadScreenManagerCleanUp);
		}
	}
}

inline void FModularLoadingScreenForStartupModule::ShutdownModule()
{
}

inline bool FModularLoadingScreenForStartupModule::IsGameModule() const
{
	return true;
}

inline void FModularLoadingScreenForStartupModule::OnPreLoadScreenManagerCleanUp()
{
	// PreLoadScreenManager 清理完毕后，移除所有资源
	PreLoadingScreen.Reset();
	ShutdownModule();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularLoadingScreenForStartupModule, ModularLoadingScreenForStartup)

/*
 * 此模块不需要暴露API,仅用于在游戏开始时自动使用模块
 * 启动加载页面
 */
