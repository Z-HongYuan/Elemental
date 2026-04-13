// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularUITags.h"

#define UE_API MODULARUI_API

namespace ModularUITags
{
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(ModularUI_UIContainer_Modal, "ModularUI.UIContainer.Modal", "UI层级的模态层");
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(ModularUI_UIContainer_GameMenu, "ModularUI.UIContainer.GameMenu", "UI层级的游戏菜单层");
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(ModularUI_UIContainer_GameHUD, "ModularUI.UIContainer.GameHUD", "UI层级的游戏HUD层");
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(ModularUI_UIContainer_Frontend, "ModularUI.UIContainer.Frontend", "UI层级的控件层");
}

#undef UE_API
