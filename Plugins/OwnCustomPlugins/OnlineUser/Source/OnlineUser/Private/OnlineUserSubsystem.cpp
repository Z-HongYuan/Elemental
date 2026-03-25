// Copyright © 2026 鸿源z. All Rights Reserved.


#include "OnlineUserSubsystem.h"

#define UE_API ONLINEUSER_API

/*注册LogOnlineUser的Log分类*/
DECLARE_LOG_CATEGORY_EXTERN(LogOnlineUser, Log, All);

/*注册LogOnlineUser的Log分类*/
DEFINE_LOG_CATEGORY(LogOnlineUser);

/*构建子系统使用的GameplayTag*/
namespace OnlineUserTags
{
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnlineUserTags::SystemMessage_Error, "SystemMessage.Error", "表示系统的错误信息")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnlineUserTags::SystemMessage_Warning, "SystemMessage.Warning", "表示系统的警告信息")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnlineUserTags::SystemMessage_Display, "SystemMessage.Display", "表示系统的显示信息")

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnlineUserTags::SystemMessage_Error_InitializeLocalPlayerFailed, "SystemMessage.Error.InitializeLocalPlayerFailed", "错误,初始化本地玩家失败")

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnlineUserTags::Platform_Trait_RequiresStrictControllerMapping, "Platform.Trait.RequiresStrictControllerMapping", "表示需要严格的控制器映射")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnlineUserTags::Platform_Trait_SingleOnlineUser, "Platform.Trait.SingleOnlineUser", "表示单一在线用户")
}


bool UOnlineUserInfo::IsLoggedIn() const
{
	return false;
}

bool UOnlineUserInfo::IsDoingLogin() const
{
	return false;
}

ECommonUserPrivilegeResult UOnlineUserInfo::GetCachedPrivilegeResult(ECommonUserPrivilege Privilege, ECommonUserOnlineContext Context) const
{
	return {};
}

ECommonUserAvailability UOnlineUserInfo::GetPrivilegeAvailability(ECommonUserPrivilege Privilege) const
{
	return {};
}

FUniqueNetIdRepl UOnlineUserInfo::GetNetId(ECommonUserOnlineContext Context) const
{
	return {};
}

FString UOnlineUserInfo::GetNickname(ECommonUserOnlineContext Context) const
{
	return {};
}

void UOnlineUserInfo::SetNickname(const FString& NewNickname, ECommonUserOnlineContext Context)
{
}

FString UOnlineUserInfo::GetDebugString() const
{
	return {};
}

FPlatformUserId UOnlineUserInfo::GetPlatformUserId() const
{
	return {};
}

int32 UOnlineUserInfo::GetPlatformUserIndex() const
{
	return 0;
}

UOnlineUserInfo::FCachedData* UOnlineUserInfo::GetCachedData(ECommonUserOnlineContext Context)
{
	return nullptr;
}

const UOnlineUserInfo::FCachedData* UOnlineUserInfo::GetCachedData(ECommonUserOnlineContext Context) const
{
	return nullptr;
}

void UOnlineUserInfo::UpdateCachedPrivilegeResult(ECommonUserPrivilege Privilege, ECommonUserPrivilegeResult Result, ECommonUserOnlineContext Context)
{
}

void UOnlineUserInfo::UpdateCachedNetId(const FUniqueNetIdRepl& NewId, ECommonUserOnlineContext Context)
{
}

class UCommonUserSubsystem* UOnlineUserInfo::GetSubsystem() const
{
	return nullptr;
}

#undef UE_API
