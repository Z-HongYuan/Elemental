// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularUserGameplayTags.h"

#define UE_API MODULARUSERMANAGEMENT_API

namespace ModularUserManagementTags
{
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(ModularUser_SystemMessage_Error, "ModularUser.SystemMessage.Error", "表示系统的错误信息")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(ModularUser_SystemMessage_Warning, "ModularUser.SystemMessage.Warning", "表示系统的警告信息")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(ModularUser_SystemMessage_Display, "ModularUser.SystemMessage.Display", "表示系统的显示信息")

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(ModularUser_SystemMessage_Error_InitializeLocalPlayerFailed, "ModularUser.SystemMessage.Error.InitializeLocalPlayerFailed",
	                                      "错误,初始化本地玩家失败")

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(ModularUser_Platform_Trait_RequiresStrictControllerMapping, "ModularUser.Platform.Trait.RequiresStrictControllerMapping",
	                                      "表示需要严格的控制器映射")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(ModularUser_Platform_Trait_SingleModularUser, "ModularUser.Platform.Trait.SingleModularUser", "表示单一在线用户")
}

#undef UE_API
