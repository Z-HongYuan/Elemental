// Copyright © 2026 鸿源z. All Rights Reserved.


#include "UserManagement/ModularUserInfo.h"
#include "UserManagement/ModularUserManager.h"

bool UModularUserInfo::IsLoggedIn() const
{
	return (InitializationState == EModularUserInitializationState::LoggedInLocalOnly || InitializationState == EModularUserInitializationState::LoggedInOnline);
}

bool UModularUserInfo::IsDoingLogin() const
{
	return (InitializationState == EModularUserInitializationState::DoingInitialLogin || InitializationState == EModularUserInitializationState::DoingNetworkLogin);
}

EModularUserPrivilegeResult UModularUserInfo::GetCachedPrivilegeResult(EModularUserPrivilege Privilege, EModularUserOnlineContext Context) const
{
	const FCachedData* FoundCached = GetCachedData(Context);

	if (FoundCached)
	{
		const EModularUserPrivilegeResult* FoundResult = FoundCached->CachedPrivileges.Find(Privilege);
		if (FoundResult)
			return *FoundResult;
	}
	return EModularUserPrivilegeResult::Unknown;
}

EModularUserAvailability UModularUserInfo::GetPrivilegeAvailability(EModularUserPrivilege Privilege) const
{
	// 快速失败,直接判断有效区间
	if ((int32)Privilege < 0 || (int32)Privilege >= (int32)EModularUserPrivilege::Invalid_Count || InitializationState == EModularUserInitializationState::Invalid)
	{
		return EModularUserAvailability::Invalid;
	}

	EModularUserPrivilegeResult CachedResult = GetCachedPrivilegeResult(Privilege, EModularUserOnlineContext::Game);

	// 首先处理显式失败
	switch (CachedResult)
	{
	case EModularUserPrivilegeResult::LicenseInvalid:
	case EModularUserPrivilegeResult::VersionOutdated:
	case EModularUserPrivilegeResult::AgeRestricted:
		return EModularUserAvailability::AlwaysUnavailable;

	case EModularUserPrivilegeResult::NetworkConnectionUnavailable:
	case EModularUserPrivilegeResult::AccountTypeRestricted:
	case EModularUserPrivilegeResult::AccountUseRestricted:
	case EModularUserPrivilegeResult::PlatformFailure:
		return EModularUserAvailability::CurrentlyUnavailable;

	default:
		break;
	}

	if (bIsGuest)
	{
		// 游客只能玩，不能使用在线功能
		if (Privilege == EModularUserPrivilege::CanPlay)
		{
			return EModularUserAvailability::NowAvailable;
		}
		else
		{
			return EModularUserAvailability::AlwaysUnavailable;
		}
	}

	// 检查网络状态
	if (Privilege == EModularUserPrivilege::CanPlayOnline ||
		Privilege == EModularUserPrivilege::CanUseCrossPlay ||
		Privilege == EModularUserPrivilege::CanCommunicateViaTextOnline ||
		Privilege == EModularUserPrivilege::CanCommunicateViaVoiceOnline)
	{
		UModularUserManager* Subsystem = GetSubsystem();
		if (ensure(Subsystem) && !Subsystem->HasOnlineConnection(EModularUserOnlineContext::Game))
		{
			return EModularUserAvailability::CurrentlyUnavailable;
		}
	}

	if (InitializationState == EModularUserInitializationState::FailedtoLogin)
	{
		// 之前的登录尝试失败
		return EModularUserAvailability::CurrentlyUnavailable;
	}
	else if (InitializationState == EModularUserInitializationState::Unknown || InitializationState == EModularUserInitializationState::DoingInitialLogin)
	{
		// 尚未登录
		return EModularUserAvailability::PossiblyAvailable;
	}
	else if (InitializationState == EModularUserInitializationState::LoggedInLocalOnly || InitializationState == EModularUserInitializationState::DoingNetworkLogin)
	{
		// 本地登录成功，因此播放检查有效
		if (Privilege == EModularUserPrivilege::CanPlay && CachedResult == EModularUserPrivilegeResult::Available)
		{
			return EModularUserAvailability::NowAvailable;
		}

		// 尚未联机登录
		return EModularUserAvailability::PossiblyAvailable;
	}
	else if (InitializationState == EModularUserInitializationState::LoggedInOnline)
	{
		// 已完全登录
		if (CachedResult == EModularUserPrivilegeResult::Available)
		{
			return EModularUserAvailability::NowAvailable;
		}

		// 因其他原因失败
		return EModularUserAvailability::CurrentlyUnavailable;
	}

	return EModularUserAvailability::Unknown;
}

FUniqueNetIdRepl UModularUserInfo::GetNetId(EModularUserOnlineContext Context) const
{
	const FCachedData* FoundCached = GetCachedData(Context);

	if (FoundCached)
		return FoundCached->CachedNetId;

	return FUniqueNetIdRepl();
}

FString UModularUserInfo::GetNickname(EModularUserOnlineContext Context) const
{
	const FCachedData* FoundCached = GetCachedData(Context);

	if (FoundCached)
		return FoundCached->CachedNickname;

	// TODO 获取需要返回一个默认昵称
	return FString();
}

void UModularUserInfo::SetNickname(const FString& NewNickname, EModularUserOnlineContext Context)
{
	FCachedData* ContextCache = GetCachedData(Context);

	if (ensure(ContextCache))
		ContextCache->CachedNickname = NewNickname;
}

FString UModularUserInfo::GetDebugString() const
{
	FUniqueNetIdRepl NetId = GetNetId();
	return NetId.ToDebugString();
}

FPlatformUserId UModularUserInfo::GetPlatformUserId() const
{
	return PlatformUser;
}

int32 UModularUserInfo::GetPlatformUserIndex() const
{
	// 将我们的平台id转换为索引
	const UModularUserManager* Subsystem = GetSubsystem();

	if (ensure(Subsystem))
		return Subsystem->GetPlatformUserIndexForId(PlatformUser);

	return INDEX_NONE;
}

UModularUserInfo::FCachedData* UModularUserInfo::GetCachedData(EModularUserOnlineContext Context)
{
	// 直接查找，游戏有一个与默认值不同的单独缓存
	FCachedData* FoundData = CachedDataMap.Find(Context);
	if (FoundData)
		return FoundData;

	// 现在尝试系统查找
	UModularUserManager* Subsystem = GetSubsystem();

	EModularUserOnlineContext ResolvedContext = Subsystem->ResolveOnlineContext(Context);
	return CachedDataMap.Find(ResolvedContext);
}

const UModularUserInfo::FCachedData* UModularUserInfo::GetCachedData(EModularUserOnlineContext Context) const
{
	return const_cast<UModularUserInfo*>(this)->GetCachedData(Context);
}

void UModularUserInfo::UpdateCachedPrivilegeResult(EModularUserPrivilege Privilege, EModularUserPrivilegeResult Result, EModularUserOnlineContext Context)
{
	// 只能使用已解析且有效的类型调用此函数
	FCachedData* GameCache = GetCachedData(EModularUserOnlineContext::Game);
	FCachedData* ContextCache = GetCachedData(Context);

	if (!ensure(GameCache && ContextCache))
	{
		// 应始终有效
		return;
	}

	// 首先更新直接缓存
	ContextCache->CachedPrivileges.Add(Privilege, Result);

	if (GameCache != ContextCache)
	{
		// 寻找另一个上下文以融入游戏
		EModularUserPrivilegeResult GameContextResult = Result;
		EModularUserPrivilegeResult OtherContextResult = EModularUserPrivilegeResult::Available;
		for (TPair<EModularUserOnlineContext, FCachedData>& Pair : CachedDataMap)
		{
			if (&Pair.Value != ContextCache && &Pair.Value != GameCache)
			{
				EModularUserPrivilegeResult* FoundResult = Pair.Value.CachedPrivileges.Find(Privilege);
				if (FoundResult)
				{
					OtherContextResult = *FoundResult;
				}
				else
				{
					OtherContextResult = EModularUserPrivilegeResult::Unknown;
				}
				break;
			}
		}

		if (GameContextResult == EModularUserPrivilegeResult::Available && OtherContextResult != EModularUserPrivilegeResult::Available)
		{
			// 其他情况更糟
			GameContextResult = OtherContextResult;
		}

		GameCache->CachedPrivileges.Add(Privilege, GameContextResult);
	}
}

void UModularUserInfo::UpdateCachedNetId(const FUniqueNetIdRepl& NewId, EModularUserOnlineContext Context)
{
	FCachedData* ContextCache = GetCachedData(Context);

	if (ensure(ContextCache))
	{
		ContextCache->CachedNetId = NewId;

		// 更新昵称
		const UModularUserManager* Subsystem = GetSubsystem();
		if (ensure(Subsystem))
		{
			if (bIsGuest)
			{
				if (ContextCache->CachedNickname.IsEmpty())
				{
					// 设置默认来宾名称（如果为空），可以使用SetNickname更改
					ContextCache->CachedNickname = NSLOCTEXT("ModularUser", "GuestNickname", "Guest").ToString();
				}
			}
			else
			{
				//使用系统昵称刷新，覆盖SetNickname
				ContextCache->CachedNickname = Subsystem->GetLocalUserNickname(GetPlatformUserId(), Context);
			}
		}
	}

	// 由于访客的工作方式，我们不会合并ID
}

class UModularUserManager* UModularUserInfo::GetSubsystem() const
{
	return Cast<UModularUserManager>(GetOuter());
}
