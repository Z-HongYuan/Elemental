// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "PreLoadScreen.h"
#include "PreLoadScreenManager.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FStartupLoadingScreenModule"

/*
 * 这个设计是为了在引擎完全初始化之前就能显示加载画面，属于底层优化方案,直接在模块加载时就有加载界面
 * 此模块启动时的加载屏幕由单独的 CommonStartupLoadingScreen 模块处理，与引擎的 PreLoadScreen 系统集成。
 * 此模块无需任何配置,当且仅当游戏启动的时候才显示开始加载界面
 * 后期更改可以变为构建好的UMG进行展示,但是又考虑到有开始影片的播放,可以考虑一下使用什么
 */
class FStartupLoadingScreenModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool IsGameModule() const override;

private:
	void OnPreLoadScreenManagerCleanUp();

	TSharedPtr<FCommonPreLoadScreen> PreLoadingScreen;
};


void FStartupLoadingScreenModule::StartupModule()
{
	// 不需要把这些资源加载到专用服务器上。
	// 仍然希望在命令行工具中加载它们，以便烹饪过程能够捕获它们
	if (!IsRunningDedicatedServer())
	{
		PreLoadingScreen = MakeShared<FCommonPreLoadScreen>();
		PreLoadingScreen->Init();

		if (!GIsEditor && FApp::CanEverRender() && FPreLoadScreenManager::Get())
		{
			FPreLoadScreenManager::Get()->RegisterPreLoadScreen(PreLoadingScreen);
			FPreLoadScreenManager::Get()->OnPreLoadScreenManagerCleanUp.AddRaw(this, &FStartupLoadingScreenModule::OnPreLoadScreenManagerCleanUp);
		}
	}
}

void FStartupLoadingScreenModule::ShutdownModule()
{
}

bool FStartupLoadingScreenModule::IsGameModule() const
{
	return true;
}

void FStartupLoadingScreenModule::OnPreLoadScreenManagerCleanUp()
{
	//一旦PreLoadScreenManager清理完毕，我们也可以清理所有资源
	PreLoadingScreen.Reset();
	ShutdownModule();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStartupLoadingScreenModule, StartupLoadingScreen)
