// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularPreLoadScreen.h"
#include "ModularPreLoadingScreenWidget.h"

#define LOCTEXT_NAMESPACE "FModularLoadingScreenForStartupModule"

void FModularPreLoadScreen::Init()
{
	/* 如果应用程序能够展示界面,并且不是在编辑器模式下,那么就加载并展示一个控件 */
	if (!GIsEditor && FApp::CanEverRender())
		EngineLoadingWidget = SNew(SModularPreLoadingScreenWidget);
}

#undef LOCTEXT_NAMESPACE
