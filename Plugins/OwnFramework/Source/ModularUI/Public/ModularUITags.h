// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

#define UE_API MODULARUI_API

namespace ModularUITags
{
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ModularUI_UIContainer_Modal);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ModularUI_UIContainer_GameMenu);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ModularUI_UIContainer_GameHUD);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ModularUI_UIContainer_Frontend);
}

#undef UE_API
