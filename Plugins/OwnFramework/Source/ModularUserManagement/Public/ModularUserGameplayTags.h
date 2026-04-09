// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

#define UE_API MODULARUSERMANAGEMENT_API

namespace ModularUserManagementTags
{
	// 常规严重程度级别和特定系统消息
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ModularUser_SystemMessage_Error);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ModularUser_SystemMessage_Warning);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ModularUser_SystemMessage_Display);

	// 所有初始化玩家的尝试均失败，用户在重试前需要执行某些操作
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ModularUser_SystemMessage_Error_InitializeLocalPlayerFailed);


	// 平台特性标签，预期游戏实例或其他系统会为适当的平台调用 SetTraitTags 并传入这些标签

	/*此标签表示这是一个主机平台，直接将控制器 ID 映射到不同的系统用户。如果为 false，则同一用户可以拥有多个控制器*/
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ModularUser_Platform_Trait_RequiresStrictControllerMapping);

	/*此标签表示该平台只有一个在线用户，所有玩家都使用索引 0*/
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(ModularUser_Platform_Trait_SingleModularUser);
}

#undef UE_API
