// Copyright © 2026 鸿源z. All Rights Reserved.

#include "PreLoadScreen.h"
#include "SPreLoadingScreenWidget.h"

#define LOCTEXT_NAMESPACE "CommonPreLoadingScreen"

void FCommonPreLoadScreen::Init()
{
	//负责创建和设置加载界面组件
	//确保不在编辑器模式下运行（只在打包后的游戏中生效）&& 检查应用程序是否支持渲染（排除无渲染的服务器环境等）
	if (!GIsEditor && FApp::CanEverRender())
	{
		// SNew(SPreLoadingScreenWidget)：使用 Unreal 的 Slate UI 系统创建新的加载屏幕控件
		// SPreLoadingScreenWidget 是在同目录下的另一个文件中定义的自定义控件
		EngineLoadingWidget = SNew(SPreLoadingScreenWidget);
	}
}

#undef LOCTEXT_NAMESPACE
