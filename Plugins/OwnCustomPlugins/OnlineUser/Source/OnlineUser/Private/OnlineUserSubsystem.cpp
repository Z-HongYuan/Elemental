// Copyright © 2026 鸿源z. All Rights Reserved.


#include "OnlineUserSubsystem.h"

#include "GameFramework/PlayerState.h"

/*根据不同的版本导入不同的头文件*/
#if ONLINEUSER_OSSV1
#include "OnlineSubsystemNames.h"
#include "OnlineSubsystemUtils.h"
#endif

#if !ONLINEUSER_OSSV1
#include "Online/Auth.h"
#include "Online/ExternalUI.h"
#include "Online/OnlineResult.h"
#include "Online/OnlineServices.h"
#include "Online/OnlineServicesEngineUtils.h"
#include "Online/Privileges.h"
using namespace UE::Online;
#endif

#define UE_API ONLINEUSER_API

/*注册LogOnlineUser的Log分类*/
DECLARE_LOG_CATEGORY_EXTERN(LogOnlineUser, Log, All);

/*注册LogOnlineUser的Log分类*/
DEFINE_LOG_CATEGORY(LogOnlineUser);

/*构建子系统使用的GameplayTag*/
namespace OnlineUserTags
{
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnlineUserTags::OnlineUser_SystemMessage_Error, "OnlineUser.SystemMessage.Error", "表示系统的错误信息")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnlineUserTags::OnlineUser_SystemMessage_Warning, "OnlineUser.SystemMessage.Warning", "表示系统的警告信息")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnlineUserTags::OnlineUser_SystemMessage_Display, "OnlineUser.SystemMessage.Display", "表示系统的显示信息")

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnlineUserTags::OnlineUser_SystemMessage_Error_InitializeLocalPlayerFailed, "OnlineUser.SystemMessage.Error.InitializeLocalPlayerFailed",
	                                      "错误,初始化本地玩家失败")

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnlineUserTags::OnlineUser_Platform_Trait_RequiresStrictControllerMapping, "OnlineUser.Platform.Trait.RequiresStrictControllerMapping",
	                                      "表示需要严格的控制器映射")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnlineUserTags::OnlineUser_Platform_Trait_SingleOnlineUser, "OnlineUser.Platform.Trait.SingleOnlineUser", "表示单一在线用户")
}

// ~ Begin UOnlineUserInfo
bool UOnlineUserInfo::IsLoggedIn() const
{
	//判断当前的玩家网络状态 已经登录本地或者已经登录网络
	return (InitializationState == EOnlineUserInitializationState::LoggedInLocalOnly || InitializationState == EOnlineUserInitializationState::LoggedInOnline);
}

bool UOnlineUserInfo::IsDoingLogin() const
{
	//判断当前的玩家网络状态 正在本地登录还是正在网络登录
	return (InitializationState == EOnlineUserInitializationState::DoingInitialLogin || InitializationState == EOnlineUserInitializationState::DoingNetworkLogin);
}

EOnlineUserPrivilegeResult UOnlineUserInfo::GetCachedPrivilegeResult(EOnlineUserPrivilege Privilege, EOnlineUserOnlineContext Context) const
{
	//获取缓存的权限结果,并且通过权限类型获取对应的权限结果
	if (const FCachedData* FoundCached = GetCachedData(Context))
		if (const EOnlineUserPrivilegeResult* FoundResult = FoundCached->CachedPrivileges.Find(Privilege))
			return *FoundResult;

	return EOnlineUserPrivilegeResult::Unknown;
}

EOnlineUserAvailability UOnlineUserInfo::GetPrivilegeAvailability(EOnlineUserPrivilege Privilege) const
{
	// 快速判断是否失败,使用强转int32判断大小是否超出范围,还有就是直接判断是否等于Invalid
	if ((int32)Privilege < 0 || (int32)Privilege >= (int32)EOnlineUserPrivilege::Invalid_Count || InitializationState == EOnlineUserInitializationState::Invalid)
	{
		return EOnlineUserAvailability::Invalid;
	}

	//从缓存中获取权限结果
	const EOnlineUserPrivilegeResult CachedResult = GetCachedPrivilegeResult(Privilege, EOnlineUserOnlineContext::Game);

	// 优先判断成功分支
	switch (CachedResult)
	{
	/*总是不可用的*/
	case EOnlineUserPrivilegeResult::LicenseInvalid:
	case EOnlineUserPrivilegeResult::VersionOutdated:
	case EOnlineUserPrivilegeResult::AgeRestricted:
		return EOnlineUserAvailability::AlwaysUnavailable;

	/*当前不可用,以后可能可用*/
	case EOnlineUserPrivilegeResult::NetworkConnectionUnavailable:
	case EOnlineUserPrivilegeResult::AccountTypeRestricted:
	case EOnlineUserPrivilegeResult::AccountUseRestricted:
	case EOnlineUserPrivilegeResult::PlatformFailure:
		return EOnlineUserAvailability::CurrentlyUnavailable;

	default:
		break;
	}

	//判断是否为访客状态
	if (bIsGuest)
	{
		// 访客只能玩，不能使用在线功能
		if (Privilege == EOnlineUserPrivilege::CanPlay)
		{
			return EOnlineUserAvailability::NowAvailable;
		}
		else
		{
			return EOnlineUserAvailability::AlwaysUnavailable;
		}
	}

	//检查与网络强相关的权限，如果当前没有网络连接，则直接返回"暂时不可用"
	if (Privilege == EOnlineUserPrivilege::CanPlayOnline ||
		Privilege == EOnlineUserPrivilege::CanUseCrossPlay ||
		Privilege == EOnlineUserPrivilege::CanCommunicateViaTextOnline ||
		Privilege == EOnlineUserPrivilege::CanCommunicateViaVoiceOnline)
	{
		UOnlineUserSubsystem* Subsystem = GetSubsystem();
		//判断没有网络连接,直接返回 "暂时不可用"
		if (ensure(Subsystem) && !Subsystem->HasOnlineConnection(EOnlineUserOnlineContext::Game))
		{
			return EOnlineUserAvailability::CurrentlyUnavailable;
		}
	}


	if (InitializationState == EOnlineUserInitializationState::FailedtoLogin)
	{
		//如果之前尝试登录失败,就直接返回 "暂时不可用"
		return EOnlineUserAvailability::CurrentlyUnavailable;
	}
	else if (InitializationState == EOnlineUserInitializationState::Unknown || InitializationState == EOnlineUserInitializationState::DoingInitialLogin)
	{
		// 证明目前处于登录中,返回"可能可用"
		return EOnlineUserAvailability::PossiblyAvailable;
	}
	else if (InitializationState == EOnlineUserInitializationState::LoggedInLocalOnly || InitializationState == EOnlineUserInitializationState::DoingNetworkLogin)
	{
		// 本地登录成功，所以权限检查是有效的
		if (Privilege == EOnlineUserPrivilege::CanPlay && CachedResult == EOnlineUserPrivilegeResult::Available)
			return EOnlineUserAvailability::NowAvailable;

		// 还没有进行网络登录
		return EOnlineUserAvailability::PossiblyAvailable;
	}
	else if (InitializationState == EOnlineUserInitializationState::LoggedInOnline)
	{
		// 完全登录
		if (CachedResult == EOnlineUserPrivilegeResult::Available)
			return EOnlineUserAvailability::NowAvailable;

		// 其他原因失败
		return EOnlineUserAvailability::CurrentlyUnavailable;
	}

	return EOnlineUserAvailability::Unknown;
}

FUniqueNetIdRepl UOnlineUserInfo::GetNetId(EOnlineUserOnlineContext Context) const
{
	if (const FCachedData* FoundCached = GetCachedData(Context))
		return FoundCached->CachedNetId;

	return FUniqueNetIdRepl();
}

FString UOnlineUserInfo::GetNickname(EOnlineUserOnlineContext Context) const
{
	if (const FCachedData* FoundCached = GetCachedData(Context))
		return FoundCached->CachedNickname;

	// TODO maybe return unknown user here?也许会做成返回"unknown"的默认用户名
	return FString();
}

void UOnlineUserInfo::SetNickname(const FString& NewNickname, EOnlineUserOnlineContext Context)
{
	FCachedData* ContextCache = GetCachedData(Context);
	if (ensure(ContextCache))
		ContextCache->CachedNickname = NewNickname;
}

FString UOnlineUserInfo::GetDebugString() const
{
	const FUniqueNetIdRepl NetId = GetNetId();
	return NetId.ToDebugString();
}

FPlatformUserId UOnlineUserInfo::GetPlatformUserId() const
{
	return PlatformUser;
}

int32 UOnlineUserInfo::GetPlatformUserIndex() const
{
	// 将我们的平台ID转换为索引
	const UOnlineUserSubsystem* Subsystem = GetSubsystem();
	if (ensure(Subsystem))
		return Subsystem->GetPlatformUserIndexForId(PlatformUser);

	return INDEX_NONE;
}

UOnlineUserInfo::FCachedData* UOnlineUserInfo::GetCachedData(EOnlineUserOnlineContext Context)
{
	// 直接查一下，游戏的缓存和默认是分开的
	// 直接通过缓存查询
	if (FCachedData* FoundData = CachedDataMap.Find(Context))
		return FoundData;


	// 如果直接找不到，使用系统解析
	UOnlineUserSubsystem* Subsystem = GetSubsystem();
	EOnlineUserOnlineContext ResolvedContext = Subsystem->ResolveOnlineContext(Context);

	return CachedDataMap.Find(ResolvedContext);
}

const UOnlineUserInfo::FCachedData* UOnlineUserInfo::GetCachedData(EOnlineUserOnlineContext Context) const
{
	return const_cast<UOnlineUserInfo*>(this)->GetCachedData(Context);
}

void UOnlineUserInfo::UpdateCachedPrivilegeResult(EOnlineUserPrivilege Privilege, EOnlineUserPrivilegeResult Result, EOnlineUserOnlineContext Context)
{
	// 只能使用已解析且有效的类型调用此函数
	FCachedData* GameCache = GetCachedData(EOnlineUserOnlineContext::Game);
	FCachedData* ContextCache = GetCachedData(Context);

	//应该总是有效的
	if (!ensure(GameCache && ContextCache)) return;

	// 先更新直接缓存
	ContextCache->CachedPrivileges.Add(Privilege, Result);

	if (GameCache != ContextCache)
	{
		// 找其他情境来融合进游戏
		EOnlineUserPrivilegeResult GameContextResult = Result;
		EOnlineUserPrivilegeResult OtherContextResult = EOnlineUserPrivilegeResult::Available;
		for (TPair<EOnlineUserOnlineContext, FCachedData>& Pair : CachedDataMap)
		{
			if (&Pair.Value != ContextCache && &Pair.Value != GameCache)
			{
				if (EOnlineUserPrivilegeResult* FoundResult = Pair.Value.CachedPrivileges.Find(Privilege))
				{
					OtherContextResult = *FoundResult;
				}
				else
				{
					OtherContextResult = EOnlineUserPrivilegeResult::Unknown;
				}
				break;
			}
		}

		if (GameContextResult == EOnlineUserPrivilegeResult::Available && OtherContextResult != EOnlineUserPrivilegeResult::Available)
		{
			// 其他背景更糟，利用那个
			GameContextResult = OtherContextResult;
		}

		GameCache->CachedPrivileges.Add(Privilege, GameContextResult);
	}
}

void UOnlineUserInfo::UpdateCachedNetId(const FUniqueNetIdRepl& NewId, EOnlineUserOnlineContext Context)
{
	FCachedData* ContextCache = GetCachedData(Context);

	if (ensure(ContextCache))
	{
		ContextCache->CachedNetId = NewId;

		// 更新昵称,使用网络
		const UOnlineUserSubsystem* Subsystem = GetSubsystem();
		if (ensure(Subsystem))
		{
			if (bIsGuest)
			{
				if (ContextCache->CachedNickname.IsEmpty())
				{
					// 如果访客名为空，可以设置默认的访客名，可以用 SetNickname 更改
					// 客人：保持自定义昵称
					ContextCache->CachedNickname = NSLOCTEXT("OnlineUser", "GuestNickname", "Guest").ToString();
				}
			}
			else
			{
				// 真实用户：使用平台提供的昵称
				ContextCache->CachedNickname = Subsystem->GetLocalUserNickname(GetPlatformUserId(), Context);
			}
		}
	}

	// 我们不会合并ID，因为使用了访客的工作流程
}

class UOnlineUserSubsystem* UOnlineUserInfo::GetSubsystem() const
{
	return Cast<UOnlineUserSubsystem>(GetOuter());
}

// ~ End UOnlineUserInfo

// ~ Begin UOnlineUserSubsystem
void UOnlineUserSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 创建自己使用的的OSS包装器
	CreateOnlineContexts();

	BindOnlineDelegates();

	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
	DeviceMapper.GetOnInputDeviceConnectionChange().AddUObject(this, &ThisClass::HandleInputDeviceConnectionChanged);

	// 匹配引擎默认配置
	SetMaxLocalPlayers(4);

	ResetUserState();

	UGameInstance* GameInstance = GetGameInstance();
	bIsDedicatedServer = GameInstance->IsDedicatedServerInstance();
}

void UOnlineUserSubsystem::Deinitialize()
{
	DestroyOnlineContexts();

	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
	DeviceMapper.GetOnInputDeviceConnectionChange().RemoveAll(this);

	LocalUserInfos.Reset();
	ActiveLoginRequests.Reset();

	Super::Deinitialize();
}

bool UOnlineUserSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	//查询子类,用继承链往下找
	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	// 只有在没有子类时才创建实例,保持单例
	return ChildClasses.Num() == 0;
}

void UOnlineUserSubsystem::SetMaxLocalPlayers(int32 InMaxLocalPlayers)
{
	if (ensure(InMaxLocalPlayers >= 1))
	{
		// 我们可以请更多本地玩家，超过上限的就当作游客对待
		MaxNumberOfLocalPlayers = InMaxLocalPlayers;

		UGameInstance* GameInstance = GetGameInstance();
		UGameViewportClient* ViewportClient = GameInstance ? GameInstance->GetGameViewportClient() : nullptr;

		//设置游戏视口的分屏上限
		if (ViewportClient)
			ViewportClient->MaxSplitscreenPlayers = MaxNumberOfLocalPlayers;
	}
}

int32 UOnlineUserSubsystem::GetMaxLocalPlayers() const
{
	return MaxNumberOfLocalPlayers;
}

int32 UOnlineUserSubsystem::GetNumLocalPlayers() const
{
	//从游戏实例中获取当前的玩家数量
	UGameInstance* GameInstance = GetGameInstance();
	if (ensure(GameInstance))
		return GameInstance->GetNumLocalPlayers();

	return 1;
}

EOnlineUserInitializationState UOnlineUserSubsystem::GetLocalPlayerInitializationState(int32 LocalPlayerIndex) const
{
	// 从用户信息中获取初始化状态
	if (const UOnlineUserInfo* UserInfo = GetUserInfoForLocalPlayerIndex(LocalPlayerIndex))
		return UserInfo->InitializationState;

	// 无效的索引
	if (LocalPlayerIndex < 0 || LocalPlayerIndex >= GetMaxLocalPlayers())
		return EOnlineUserInitializationState::Invalid;

	return EOnlineUserInitializationState::Unknown;
}

const UOnlineUserInfo* UOnlineUserSubsystem::GetUserInfoForLocalPlayerIndex(int32 LocalPlayerIndex) const
{
	if (TObjectPtr<UOnlineUserInfo> const* Found = LocalUserInfos.Find(LocalPlayerIndex))
		return *Found;
	return nullptr;
}

const UOnlineUserInfo* UOnlineUserSubsystem::GetUserInfoForPlatformUserIndex(int32 PlatformUserIndex) const
{
	FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);
	return GetUserInfoForPlatformUser(PlatformUser);
}

const UOnlineUserInfo* UOnlineUserSubsystem::GetUserInfoForPlatformUser(FPlatformUserId PlatformUser) const
{
	if (!IsRealPlatformUser(PlatformUser)) return nullptr;

	// 检查确保没有游客用户
	for (TPair<int32, UOnlineUserInfo*> Pair : LocalUserInfos)
	{
		if (ensure(Pair.Value) && Pair.Value->PlatformUser == PlatformUser && !Pair.Value->bIsGuest)
			return Pair.Value;
	}
	return nullptr;
}

const UOnlineUserInfo* UOnlineUserSubsystem::GetUserInfoForUniqueNetId(const FUniqueNetIdRepl& NetId) const
{
	if (!NetId.IsValid())
		// TODO do we need to handle pre-login case on mobile platforms where netID is invalid? 在移动平台NetID无效时，我们是否需要处理预登录案件？
		return nullptr;

	//从每个索引中检查
	for (TPair<int32, UOnlineUserInfo*> UserPair : LocalUserInfos)
	{
		if (ensure(UserPair.Value))
		{
			for (const TPair<EOnlineUserOnlineContext, UOnlineUserInfo::FCachedData>& CachedPair : UserPair.Value->CachedDataMap)
			{
				if (NetId == CachedPair.Value.CachedNetId) return UserPair.Value;
			}
		}
	}

	return nullptr;
}

const UOnlineUserInfo* UOnlineUserSubsystem::GetUserInfoForControllerId(int32 ControllerId) const
{
	FPlatformUserId PlatformUser;
	FInputDeviceId IgnoreDevice;
	IPlatformInputDeviceMapper::Get().RemapControllerIdToPlatformUserAndDevice(ControllerId, PlatformUser, IgnoreDevice);
	return GetUserInfoForPlatformUser(PlatformUser);
}

const UOnlineUserInfo* UOnlineUserSubsystem::GetUserInfoForInputDevice(FInputDeviceId InputDevice) const
{
	FPlatformUserId PlatformUser = GetPlatformUserIdForInputDevice(InputDevice);
	return GetUserInfoForPlatformUser(PlatformUser);
}

bool UOnlineUserSubsystem::TryToInitializeForLocalPlay(int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin)
{
	// 设置为默认设备
	if (!PrimaryInputDevice.IsValid())
		PrimaryInputDevice = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();

	//使用参数初始化用户
	FOnlineUserInitializeParams Params;
	Params.LocalPlayerIndex = LocalPlayerIndex;
	Params.PrimaryInputDevice = PrimaryInputDevice;
	Params.bCanUseGuestLogin = bCanUseGuestLogin;
	Params.bCanCreateNewLocalPlayer = true;
	Params.RequestedPrivilege = EOnlineUserPrivilege::CanPlay;

	return TryToInitializeUser(Params);
}

bool UOnlineUserSubsystem::TryToLoginForOnlinePlay(int32 LocalPlayerIndex)
{
	//使用参数初始化用户
	FOnlineUserInitializeParams Params;
	Params.LocalPlayerIndex = LocalPlayerIndex;
	Params.bCanCreateNewLocalPlayer = false;
	Params.RequestedPrivilege = EOnlineUserPrivilege::CanPlayOnline;

	return TryToInitializeUser(Params);
}

bool UOnlineUserSubsystem::TryToInitializeUser(FOnlineUserInitializeParams Params)
{
	//快速判断,索引有效性
	if (Params.LocalPlayerIndex < 0 || (!Params.bCanCreateNewLocalPlayer && Params.LocalPlayerIndex >= GetNumLocalPlayers()))
	{
		if (!bIsDedicatedServer)
		{
			UE_LOG(LogOnlineUser, Error, TEXT("TryToInitializeUser %d failed with current %d and max %d, invalid index"),
			       Params.LocalPlayerIndex, GetNumLocalPlayers(), GetMaxLocalPlayers());
			return false;
		}
	}

	//判断玩家的最大数量
	if (Params.LocalPlayerIndex > GetNumLocalPlayers() || Params.LocalPlayerIndex >= GetMaxLocalPlayers())
	{
		UE_LOG(LogOnlineUser, Error, TEXT("TryToInitializeUser %d failed with current %d and max %d, can only create in order up to max players"),
		       Params.LocalPlayerIndex, GetNumLocalPlayers(), GetMaxLocalPlayers());
		return false;
	}

	// 如果需要，填写平台用户和输入设备
	if (Params.ControllerId != INDEX_NONE && (!Params.PrimaryInputDevice.IsValid() || !Params.PlatformUser.IsValid()))
	{
		IPlatformInputDeviceMapper::Get().RemapControllerIdToPlatformUserAndDevice(Params.ControllerId, Params.PlatformUser, Params.PrimaryInputDevice);
	}

	if (Params.PrimaryInputDevice.IsValid() && !Params.PlatformUser.IsValid())
	{
		Params.PlatformUser = GetPlatformUserIdForInputDevice(Params.PrimaryInputDevice);
	}
	else if (Params.PlatformUser.IsValid() && !Params.PrimaryInputDevice.IsValid())
	{
		Params.PrimaryInputDevice = GetPrimaryInputDeviceForPlatformUser(Params.PlatformUser);
	}

	UOnlineUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(Params.LocalPlayerIndex));
	UOnlineUserInfo* LocalUserInfoForController = ModifyInfo(GetUserInfoForInputDevice(Params.PrimaryInputDevice));

	if (LocalUserInfoForController && LocalUserInfo && LocalUserInfoForController != LocalUserInfo)
	{
		UE_LOG(LogOnlineUser, Error, TEXT("TryToInitializeUser %d failed because controller %d is already assigned to player %d"),
		       Params.LocalPlayerIndex, Params.PrimaryInputDevice.GetId(), LocalUserInfoForController->LocalPlayerIndex);
		return false;
	}

	if (Params.LocalPlayerIndex == 0 && Params.bCanUseGuestLogin)
	{
		UE_LOG(LogOnlineUser, Error, TEXT("TryToInitializeUser failed because player 0 cannot be a guest"));
		return false;
	}

	if (!LocalUserInfo)
	{
		LocalUserInfo = CreateLocalUserInfo(Params.LocalPlayerIndex);
	}
	else
	{
		// 从现有用户信息复制
		if (!Params.PrimaryInputDevice.IsValid())
		{
			Params.PrimaryInputDevice = LocalUserInfo->PrimaryInputDevice;
		}

		if (!Params.PlatformUser.IsValid())
		{
			Params.PlatformUser = LocalUserInfo->PlatformUser;
		}
	}

	if (LocalUserInfo->InitializationState != EOnlineUserInitializationState::Unknown && LocalUserInfo->InitializationState != EOnlineUserInitializationState::FailedtoLogin)
	{
		// 登录时不允许更改参数
		if (LocalUserInfo->PrimaryInputDevice != Params.PrimaryInputDevice || LocalUserInfo->PlatformUser != Params.PlatformUser || LocalUserInfo->bCanBeGuest != Params.
			bCanUseGuestLogin)
		{
			UE_LOG(LogOnlineUser, Error, TEXT("TryToInitializeUser failed because player %d has already started the login process with diffrent settings!"),
			       Params.LocalPlayerIndex);
			return false;
		}
	}

	// 现在设置目标索引，这样如果创建播放器时它知道该用哪个控制器
	LocalUserInfo->PrimaryInputDevice = Params.PrimaryInputDevice;
	LocalUserInfo->PlatformUser = Params.PlatformUser;
	LocalUserInfo->bCanBeGuest = Params.bCanUseGuestLogin;
	RefreshLocalUserInfo(LocalUserInfo);

	// 无论是初次登录还是网络登录
	if (LocalUserInfo->GetPrivilegeAvailability(EOnlineUserPrivilege::CanPlay) == EOnlineUserAvailability::NowAvailable && Params.RequestedPrivilege ==
		EOnlineUserPrivilege::CanPlayOnline)
	{
		LocalUserInfo->InitializationState = EOnlineUserInitializationState::DoingNetworkLogin;
	}
	else
	{
		LocalUserInfo->InitializationState = EOnlineUserInitializationState::DoingInitialLogin;
	}

	LoginLocalUser(LocalUserInfo, Params.RequestedPrivilege, Params.OnlineContext,
	               FOnLocalUserLoginCompleteDelegate::CreateUObject(this, &ThisClass::HandleLoginForUserInitialize, Params));

	return true;
}

void UOnlineUserSubsystem::ListenForLoginKeyInput(TArray<FKey> AnyUserKeys, TArray<FKey> NewUserKeys, FOnlineUserInitializeParams Params)
{
	UGameViewportClient* ViewportClient = GetGameInstance()->GetGameViewportClient();
	if (ensure(ViewportClient))
	{
		const bool bIsMapped = LoginKeysForAnyUser.Num() > 0 || LoginKeysForNewUser.Num() > 0;
		const bool bShouldBeMapped = AnyUserKeys.Num() > 0 || NewUserKeys.Num() > 0;

		if (bIsMapped && !bShouldBeMapped)
		{
			// Set it back to wrapped handler
			ViewportClient->OnOverrideInputKey() = WrappedInputKeyHandler;
			WrappedInputKeyHandler.Unbind();
		}
		else if (!bIsMapped && bShouldBeMapped)
		{
			// Set up a wrapped handler
			WrappedInputKeyHandler = ViewportClient->OnOverrideInputKey();
			ViewportClient->OnOverrideInputKey().BindUObject(this, &UOnlineUserSubsystem::OverrideInputKeyForLogin);
		}

		LoginKeysForAnyUser = AnyUserKeys;
		LoginKeysForNewUser = NewUserKeys;

		if (bShouldBeMapped)
		{
			ParamsForLoginKey = Params;
		}
		else
		{
			ParamsForLoginKey = FOnlineUserInitializeParams();
		}
	}
}

bool UOnlineUserSubsystem::CancelUserInitialization(int32 LocalPlayerIndex)
{
	UOnlineUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(LocalPlayerIndex));
	if (!LocalUserInfo)
	{
		return false;
	}

	if (!LocalUserInfo->IsDoingLogin())
	{
		return false;
	}

	// Remove from login queue
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		if (Request->UserInfo.IsValid() && Request->UserInfo->LocalPlayerIndex == LocalPlayerIndex)
		{
			ActiveLoginRequests.Remove(Request);
		}
	}

	// Set state with best guess
	if (LocalUserInfo->InitializationState == EOnlineUserInitializationState::DoingNetworkLogin)
	{
		LocalUserInfo->InitializationState = EOnlineUserInitializationState::LoggedInLocalOnly;
	}
	else
	{
		LocalUserInfo->InitializationState = EOnlineUserInitializationState::FailedtoLogin;
	}

	return true;
}

bool UOnlineUserSubsystem::TryToLogOutUser(int32 LocalPlayerIndex, bool bDestroyPlayer)
{
	UGameInstance* GameInstance = GetGameInstance();

	if (!ensure(GameInstance))
	{
		return false;
	}

	if (LocalPlayerIndex == 0 && bDestroyPlayer)
	{
		UE_LOG(LogOnlineUser, Error, TEXT("TryToLogOutUser cannot destroy player 0"));
		return false;
	}

	CancelUserInitialization(LocalPlayerIndex);

	UOnlineUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(LocalPlayerIndex));
	if (!LocalUserInfo)
	{
		UE_LOG(LogOnlineUser, Warning, TEXT("TryToLogOutUser failed to log out user %i because they are not logged in"), LocalPlayerIndex);
		return false;
	}

	FPlatformUserId UserId = LocalUserInfo->PlatformUser;
	if (IsRealPlatformUser(UserId))
	{
		// Currently this does not do platform logout in case they want to log back in immediately after
		UE_LOG(LogOnlineUser, Log, TEXT("TryToLogOutUser succeeded for real platform user %d"), UserId.GetInternalId());

		LogOutLocalUser(UserId);
	}
	else if (ensure(LocalUserInfo->bIsGuest))
	{
		// For guest users just delete it
		UE_LOG(LogOnlineUser, Log, TEXT("TryToLogOutUser succeeded for guest player index %d"), LocalPlayerIndex);

		LocalUserInfos.Remove(LocalPlayerIndex);
	}

	if (bDestroyPlayer)
	{
		ULocalPlayer* ExistingPlayer = GameInstance->FindLocalPlayerFromPlatformUserId(UserId);

		if (ExistingPlayer)
		{
			GameInstance->RemoveLocalPlayer(ExistingPlayer);
		}
	}

	return true;
}

void UOnlineUserSubsystem::ResetUserState()
{
	// Manually purge existing info objects
	for (TPair<int32, UOnlineUserInfo*> Pair : LocalUserInfos)
	{
		if (Pair.Value)
		{
			Pair.Value->MarkAsGarbage();
		}
	}

	LocalUserInfos.Reset();

	// Cancel in-progress logins
	ActiveLoginRequests.Reset();

	// Create player info for id 0
	UOnlineUserInfo* FirstUser = CreateLocalUserInfo(0);

	FirstUser->PlatformUser = IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser();
	FirstUser->PrimaryInputDevice = IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(FirstUser->PlatformUser);

	// TODO: Schedule a refresh of player 0 for next frame?
	RefreshLocalUserInfo(FirstUser);
}

bool UOnlineUserSubsystem::IsRealPlatformUserIndex(int32 PlatformUserIndex) const
{
	if (PlatformUserIndex < 0)
	{
		return false;
	}

#if ONLINEUSER_OSSV1
	if (PlatformUserIndex >= MAX_LOCAL_PLAYERS)
	{
		// Check against OSS count
		return false;
	}
#else
	// TODO:  OSSv2 define MAX_LOCAL_PLAYERS?
#endif

	if (PlatformUserIndex > 0 && GetTraitTags().HasTag(OnlineUserTags::OnlineUser_Platform_Trait_SingleOnlineUser))
	{
		return false;
	}

	return true;
}

bool UOnlineUserSubsystem::IsRealPlatformUser(FPlatformUserId PlatformUser) const
{
	// Validation is done at conversion/allocation time so trust the type
	if (!PlatformUser.IsValid())
	{
		return false;
	}

	// TODO: Validate against OSS or input mapper somehow

	if (GetTraitTags().HasTag(OnlineUserTags::OnlineUser_Platform_Trait_SingleOnlineUser))
	{
		// Only the default user is supports online functionality 
		if (PlatformUser != IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser())
		{
			return false;
		}
	}

	return true;
}

FPlatformUserId UOnlineUserSubsystem::GetPlatformUserIdForIndex(int32 PlatformUserIndex) const
{
	return IPlatformInputDeviceMapper::Get().GetPlatformUserForUserIndex(PlatformUserIndex);
}

int32 UOnlineUserSubsystem::GetPlatformUserIndexForId(FPlatformUserId PlatformUser) const
{
	return IPlatformInputDeviceMapper::Get().GetUserIndexForPlatformUser(PlatformUser);
}

FPlatformUserId UOnlineUserSubsystem::GetPlatformUserIdForInputDevice(FInputDeviceId InputDevice) const
{
	return IPlatformInputDeviceMapper::Get().GetUserForInputDevice(InputDevice);
}

FInputDeviceId UOnlineUserSubsystem::GetPrimaryInputDeviceForPlatformUser(FPlatformUserId PlatformUser) const
{
	return IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(PlatformUser);
}

void UOnlineUserSubsystem::SetTraitTags(const FGameplayTagContainer& InTags)
{
	CachedTraitTags = InTags;
}

bool UOnlineUserSubsystem::ShouldWaitForStartInput() const
{
	// By default, don't wait for input if this is a single user platform
	return !HasTraitTag(OnlineUserTags::OnlineUser_Platform_Trait_SingleOnlineUser.GetTag());
}

bool UOnlineUserSubsystem::HasOnlineConnection(EOnlineUserOnlineContext Context) const
{
#if ONLINEUSER_OSSV1
	EOnlineServerConnectionStatus::Type ConnectionType = GetConnectionStatus(Context);

	if (ConnectionType == EOnlineServerConnectionStatus::Normal || ConnectionType == EOnlineServerConnectionStatus::Connected)
	{
		return true;
	}

	return false;
#else
	return GetConnectionStatus(Context) == UE::Online::EOnlineServicesConnectionStatus::Connected;
#endif
}

ELoginStatusType UOnlineUserSubsystem::GetLocalUserLoginStatus(FPlatformUserId PlatformUser, EOnlineUserOnlineContext Context) const
{
	if (!IsRealPlatformUser(PlatformUser))
	{
		return ELoginStatusType::NotLoggedIn;
	}

	const FOnlineContextCache* System = GetContextCache(Context);
	if (System)
	{
#if ONLINEUSER_OSSV1
		return System->IdentityInterface->GetLoginStatus(GetPlatformUserIndexForId(PlatformUser));
#else
		if (TSharedPtr<FAccountInfo> AccountInfo = GetOnlineServiceAccountInfo(System->AuthService, PlatformUser))
		{
			return AccountInfo->LoginStatus;
		}
#endif
	}
	return ELoginStatusType::NotLoggedIn;
}

FUniqueNetIdRepl UOnlineUserSubsystem::GetLocalUserNetId(FPlatformUserId PlatformUser, EOnlineUserOnlineContext Context) const
{
	if (!IsRealPlatformUser(PlatformUser))
	{
		return FUniqueNetIdRepl();
	}

	const FOnlineContextCache* System = GetContextCache(Context);
	if (System)
	{
#if ONLINEUSER_OSSV1
		return FUniqueNetIdRepl(System->IdentityInterface->GetUniquePlayerId(GetPlatformUserIndexForId(PlatformUser)));
#else
		// TODO:  OSSv2 FUniqueNetIdRepl wrapping FAccountId is in progress
		if (TSharedPtr<FAccountInfo> AccountInfo = GetOnlineServiceAccountInfo(System->AuthService, PlatformUser))
		{
			return FUniqueNetIdRepl(AccountInfo->AccountId);
		}
#endif
	}

	return FUniqueNetIdRepl();
}

FString UOnlineUserSubsystem::GetLocalUserNickname(FPlatformUserId PlatformUser, EOnlineUserOnlineContext Context) const
{
#if ONLINEUSER_OSSV1
	IOnlineIdentity* Identity = GetOnlineIdentity(Context);
	if (ensure(Identity))
	{
		return Identity->GetPlayerNickname(GetPlatformUserIndexForId(PlatformUser));
	}
#else
	if (IAuthPtr AuthService = GetOnlineAuth(Context))
	{
		if (TSharedPtr<FAccountInfo> AccountInfo = GetOnlineServiceAccountInfo(AuthService, PlatformUser))
		{
			if (const FSchemaVariant* DisplayName = AccountInfo->Attributes.Find(AccountAttributeData::DisplayName))
			{
				return DisplayName->GetString();
			}
		}
	}
#endif // OnlineUSER_OSSV1

	return FString();
}

FString UOnlineUserSubsystem::PlatformUserIdToString(FPlatformUserId UserId)
{
	if (UserId == PLATFORMUSERID_NONE)
		return TEXT("None");
	return FString::Printf(TEXT("%d"), UserId.GetInternalId());
}

FString UOnlineUserSubsystem::EOnlineUserOnlineContextToString(EOnlineUserOnlineContext Context)
{
	switch (Context)
	{
	case EOnlineUserOnlineContext::Game:
		return TEXT("Game");
	case EOnlineUserOnlineContext::Default:
		return TEXT("Default");
	case EOnlineUserOnlineContext::Service:
		return TEXT("Service");
	case EOnlineUserOnlineContext::ServiceOrDefault:
		return TEXT("Service/Default");
	case EOnlineUserOnlineContext::Platform:
		return TEXT("Platform");
	case EOnlineUserOnlineContext::PlatformOrDefault:
		return TEXT("Platform/Default");
	default:
		return TEXT("Invalid");
	}
}

FText UOnlineUserSubsystem::GetPrivilegeDescription(EOnlineUserPrivilege Privilege) const
{
	switch (Privilege)
	{
	case EOnlineUserPrivilege::CanPlay:
		return NSLOCTEXT("OnlineUser", "PrivilegeCanPlay", "play the game");
	case EOnlineUserPrivilege::CanPlayOnline:
		return NSLOCTEXT("OnlineUser", "PrivilegeCanPlayOnline", "play online");
	case EOnlineUserPrivilege::CanCommunicateViaTextOnline:
		return NSLOCTEXT("OnlineUser", "PrivilegeCanCommunicateViaTextOnline", "communicate with text");
	case EOnlineUserPrivilege::CanCommunicateViaVoiceOnline:
		return NSLOCTEXT("OnlineUser", "PrivilegeCanCommunicateViaVoiceOnline", "communicate with voice");
	case EOnlineUserPrivilege::CanUseUserGeneratedContent:
		return NSLOCTEXT("OnlineUser", "PrivilegeCanUseUserGeneratedContent", "access user content");
	case EOnlineUserPrivilege::CanUseCrossPlay:
		return NSLOCTEXT("OnlineUser", "PrivilegeCanUseCrossPlay", "play with other platforms");
	default:
		return NSLOCTEXT("OnlineUser", "PrivilegeInvalid", "Invalid");
	}
}

FText UOnlineUserSubsystem::GetPrivilegeResultDescription(EOnlineUserPrivilegeResult Result) const
{
	// TODO these strings might have cert requirements we need to override per console
	switch (Result)
	{
	case EOnlineUserPrivilegeResult::Unknown:
		return NSLOCTEXT("OnlineUser", "ResultUnknown", "Unknown if the user is allowed");
	case EOnlineUserPrivilegeResult::Available:
		return NSLOCTEXT("OnlineUser", "ResultAvailable", "The user is allowed");
	case EOnlineUserPrivilegeResult::UserNotLoggedIn:
		return NSLOCTEXT("OnlineUser", "ResultUserNotLoggedIn", "The user must login");
	case EOnlineUserPrivilegeResult::LicenseInvalid:
		return NSLOCTEXT("OnlineUser", "ResultLicenseInvalid", "A valid game license is required");
	case EOnlineUserPrivilegeResult::VersionOutdated:
		return NSLOCTEXT("OnlineUser", "VersionOutdated", "The game or hardware needs to be updated");
	case EOnlineUserPrivilegeResult::NetworkConnectionUnavailable:
		return NSLOCTEXT("OnlineUser", "ResultNetworkConnectionUnavailable", "A network connection is required");
	case EOnlineUserPrivilegeResult::AgeRestricted:
		return NSLOCTEXT("OnlineUser", "ResultAgeRestricted", "This age restricted account is not allowed");
	case EOnlineUserPrivilegeResult::AccountTypeRestricted:
		return NSLOCTEXT("OnlineUser", "ResultAccountTypeRestricted", "This account type does not have access");
	case EOnlineUserPrivilegeResult::AccountUseRestricted:
		return NSLOCTEXT("OnlineUser", "ResultAccountUseRestricted", "This account is not allowed");
	case EOnlineUserPrivilegeResult::PlatformFailure:
		return NSLOCTEXT("OnlineUser", "ResultPlatformFailure", "Not allowed");
	default:
		return NSLOCTEXT("OnlineUser", "ResultInvalid", "Invalid");
	}
}

bool UOnlineUserSubsystem::LoginLocalUser(const UOnlineUserInfo* UserInfo, EOnlineUserPrivilege RequestedPrivilege, EOnlineUserOnlineContext Context,
                                          FOnLocalUserLoginCompleteDelegate OnComplete)
{
	UOnlineUserInfo* LocalUserInfo = ModifyInfo(UserInfo);
	if (!ensure(UserInfo))
	{
		return false;
	}

	TSharedRef<FUserLoginRequest> NewRequest = MakeShared<FUserLoginRequest>(LocalUserInfo, RequestedPrivilege, Context, MoveTemp(OnComplete));
	ActiveLoginRequests.Add(NewRequest);

	// This will execute callback or start login process
	ProcessLoginRequest(NewRequest);

	return true;
}

void UOnlineUserSubsystem::SetLocalPlayerUserInfo(ULocalPlayer* LocalPlayer, const UOnlineUserInfo* UserInfo)
{
	if (!bIsDedicatedServer && ensure(LocalPlayer && UserInfo))
	{
		LocalPlayer->SetPlatformUserId(UserInfo->GetPlatformUserId());

		FUniqueNetIdRepl NetId = UserInfo->GetNetId(EOnlineUserOnlineContext::Game);
		LocalPlayer->SetCachedUniqueNetId(NetId);

		// Also update player state if possible
		APlayerController* PlayerController = LocalPlayer->GetPlayerController(nullptr);
		if (PlayerController && PlayerController->PlayerState)
		{
			PlayerController->PlayerState->SetUniqueId(NetId);
		}
	}
}

EOnlineUserOnlineContext UOnlineUserSubsystem::ResolveOnlineContext(EOnlineUserOnlineContext Context) const
{
	switch (Context)
	{
	case EOnlineUserOnlineContext::Game:
	case EOnlineUserOnlineContext::Default:
		return EOnlineUserOnlineContext::Default;

	case EOnlineUserOnlineContext::Service:
		return ServiceContextInternal ? EOnlineUserOnlineContext::Service : EOnlineUserOnlineContext::Invalid;
	case EOnlineUserOnlineContext::ServiceOrDefault:
		return ServiceContextInternal ? EOnlineUserOnlineContext::Service : EOnlineUserOnlineContext::Default;

	case EOnlineUserOnlineContext::Platform:
		return PlatformContextInternal ? EOnlineUserOnlineContext::Platform : EOnlineUserOnlineContext::Invalid;
	case EOnlineUserOnlineContext::PlatformOrDefault:
		return PlatformContextInternal ? EOnlineUserOnlineContext::Platform : EOnlineUserOnlineContext::Default;
	}

	return EOnlineUserOnlineContext::Invalid;
}

bool UOnlineUserSubsystem::HasSeparatePlatformContext() const
{
	EOnlineUserOnlineContext ServiceType = ResolveOnlineContext(EOnlineUserOnlineContext::ServiceOrDefault);
	EOnlineUserOnlineContext PlatformType = ResolveOnlineContext(EOnlineUserOnlineContext::PlatformOrDefault);

	if (ServiceType != PlatformType)
		return true;

	return false;
}

UOnlineUserInfo* UOnlineUserSubsystem::CreateLocalUserInfo(int32 LocalPlayerIndex)
{
	UOnlineUserInfo* NewUser = nullptr;
	if (ensure(!LocalUserInfos.Contains(LocalPlayerIndex)))
	{
		NewUser = NewObject<UOnlineUserInfo>(this);
		NewUser->LocalPlayerIndex = LocalPlayerIndex;
		NewUser->InitializationState = EOnlineUserInitializationState::Unknown;

		// 始终创建游戏和默认缓存
		NewUser->CachedDataMap.Add(EOnlineUserOnlineContext::Game, UOnlineUserInfo::FCachedData());
		NewUser->CachedDataMap.Add(EOnlineUserOnlineContext::Default, UOnlineUserInfo::FCachedData());

		// 如果需要，添加平台
		if (HasSeparatePlatformContext())
			NewUser->CachedDataMap.Add(EOnlineUserOnlineContext::Platform, UOnlineUserInfo::FCachedData());

		LocalUserInfos.Add(LocalPlayerIndex, NewUser);
	}
	return NewUser;
}

void UOnlineUserSubsystem::RefreshLocalUserInfo(UOnlineUserInfo* UserInfo)
{
	if (ensure(UserInfo))
	{
		// Always update default
		UserInfo->UpdateCachedNetId(GetLocalUserNetId(UserInfo->PlatformUser, EOnlineUserOnlineContext::Default), EOnlineUserOnlineContext::Default);

		if (HasSeparatePlatformContext())
			// Also update platform
			UserInfo->UpdateCachedNetId(GetLocalUserNetId(UserInfo->PlatformUser, EOnlineUserOnlineContext::Platform), EOnlineUserOnlineContext::Platform);
	}
}

void UOnlineUserSubsystem::HandleChangedAvailability(UOnlineUserInfo* UserInfo, EOnlineUserPrivilege Privilege, EOnlineUserAvailability OldAvailability)
{
	EOnlineUserAvailability NewAvailability = UserInfo->GetPrivilegeAvailability(Privilege);

	if (OldAvailability != NewAvailability)
		OnUserPrivilegeChanged.Broadcast(UserInfo, Privilege, OldAvailability, NewAvailability);
}

void UOnlineUserSubsystem::UpdateUserPrivilegeResult(UOnlineUserInfo* UserInfo, EOnlineUserPrivilege Privilege, EOnlineUserPrivilegeResult Result, EOnlineUserOnlineContext Context)
{
	check(UserInfo);

	EOnlineUserAvailability OldAvailability = UserInfo->GetPrivilegeAvailability(Privilege);

	UserInfo->UpdateCachedPrivilegeResult(Privilege, Result, Context);

	HandleChangedAvailability(UserInfo, Privilege, OldAvailability);
}

const UOnlineUserSubsystem::FOnlineContextCache* UOnlineUserSubsystem::GetContextCache(EOnlineUserOnlineContext Context) const
{
	return const_cast<UOnlineUserSubsystem*>(this)->GetContextCache(Context);
}

UOnlineUserSubsystem::FOnlineContextCache* UOnlineUserSubsystem::GetContextCache(EOnlineUserOnlineContext Context)
{
	switch (Context)
	{
	case EOnlineUserOnlineContext::Game:
	case EOnlineUserOnlineContext::Default:
		return DefaultContextInternal;

	case EOnlineUserOnlineContext::Service:
		return ServiceContextInternal;
	case EOnlineUserOnlineContext::ServiceOrDefault:
		return ServiceContextInternal ? ServiceContextInternal : DefaultContextInternal;

	case EOnlineUserOnlineContext::Platform:
		return PlatformContextInternal;
	case EOnlineUserOnlineContext::PlatformOrDefault:
		return PlatformContextInternal ? PlatformContextInternal : DefaultContextInternal;
	}

	return nullptr;
}

void UOnlineUserSubsystem::CreateOnlineContexts()
{
	// 首先初始化默认
	DefaultContextInternal = new FOnlineContextCache();

#if ONLINEUSER_OSSV1
	DefaultContextInternal->OnlineSubsystem = Online::GetSubsystem(GetWorld());
	check(DefaultContextInternal->OnlineSubsystem);
	DefaultContextInternal->IdentityInterface = DefaultContextInternal->OnlineSubsystem->GetIdentityInterface();
	check(DefaultContextInternal->IdentityInterface.IsValid());

	IOnlineSubsystem* PlatformSub = IOnlineSubsystem::GetByPlatform();

	if (PlatformSub && DefaultContextInternal->OnlineSubsystem != PlatformSub)
	{
		// 如果有可选平台服务，请设置
		PlatformContextInternal = new FOnlineContextCache();
		PlatformContextInternal->OnlineSubsystem = PlatformSub;
		PlatformContextInternal->IdentityInterface = PlatformSub->GetIdentityInterface();
		check(PlatformContextInternal->IdentityInterface.IsValid());
	}
#else
	DefaultContextInternal->OnlineServices = GetServices(GetWorld(), EOnlineServices::Default);
	check(DefaultContextInternal->OnlineServices);
	DefaultContextInternal->AuthService = DefaultContextInternal->OnlineServices->GetAuthInterface();
	check(DefaultContextInternal->AuthService);

	UE::Online::IOnlineServicesPtr PlatformServices = GetServices(GetWorld(), EOnlineServices::Platform);
	if (PlatformServices && DefaultContextInternal->OnlineServices != PlatformServices)
	{
		PlatformContextInternal = new FOnlineContextCache();
		PlatformContextInternal->OnlineServices = PlatformServices;
		PlatformContextInternal->AuthService = PlatformContextInternal->OnlineServices->GetAuthInterface();
		check(PlatformContextInternal->AuthService);
	}
#endif

	// 如果需要，可以之后设置显式外部服务
}

void UOnlineUserSubsystem::DestroyOnlineContexts()
{
	// 必须清除所有的缓存指针
	if (ServiceContextInternal && ServiceContextInternal != DefaultContextInternal)
	{
		delete ServiceContextInternal;
	}
	if (PlatformContextInternal && PlatformContextInternal != DefaultContextInternal)
	{
		delete PlatformContextInternal;
	}
	if (DefaultContextInternal)
	{
		delete DefaultContextInternal;
	}

	ServiceContextInternal = PlatformContextInternal = DefaultContextInternal = nullptr;
}

void UOnlineUserSubsystem::BindOnlineDelegates()
{
#if ONLINEUSER_OSSV1
	return BindOnlineDelegatesOSSv1();
#else
	return BindOnlineDelegatesOSSv2();
#endif
}

void UOnlineUserSubsystem::LogOutLocalUser(FPlatformUserId PlatformUser)
{
	UOnlineUserInfo* UserInfo = ModifyInfo(GetUserInfoForPlatformUser(PlatformUser));

	// 如果用户从未完全登录或正在登录，则无需执行任何操作
	if (UserInfo && (UserInfo->InitializationState == EOnlineUserInitializationState::LoggedInLocalOnly || UserInfo->InitializationState ==
		EOnlineUserInitializationState::LoggedInOnline))
	{
		EOnlineUserAvailability OldAvailablity = UserInfo->GetPrivilegeAvailability(EOnlineUserPrivilege::CanPlay);

		UserInfo->InitializationState = EOnlineUserInitializationState::FailedtoLogin;

		// 这将广播游戏代表
		HandleChangedAvailability(UserInfo, EOnlineUserPrivilege::CanPlay, OldAvailablity);
	}
}

void UOnlineUserSubsystem::ProcessLoginRequest(TSharedRef<FUserLoginRequest> Request)
{
	// First, see if we've fully logged in
	UOnlineUserInfo* UserInfo = Request->UserInfo.Get();

	if (!UserInfo)
	{
		// User is gone, just delete this request
		ActiveLoginRequests.Remove(Request);

		return;
	}

	const FPlatformUserId PlatformUser = UserInfo->GetPlatformUserId();

	// If the platform user id is invalid because this is a guest, skip right to failure
	if (!IsRealPlatformUser(PlatformUser))
	{
#if ONLINEUSER_OSSV1
		Request->Error = FOnlineError(NSLOCTEXT("OnlineUser", "InvalidPlatformUser", "Invalid Platform User"));
#else
		Request->Error = UE::Online::Errors::InvalidUser();
#endif
		// Remove from active array
		ActiveLoginRequests.Remove(Request);

		// Execute delegate if bound
		Request->Delegate.ExecuteIfBound(UserInfo, ELoginStatusType::NotLoggedIn, FUniqueNetIdRepl(), Request->Error, Request->DesiredContext);

		return;
	}

	// Figure out what context to process first
	if (Request->CurrentContext == EOnlineUserOnlineContext::Invalid)
	{
		// First start with platform context if this is a game login
		if (Request->DesiredContext == EOnlineUserOnlineContext::Game)
		{
			Request->CurrentContext = ResolveOnlineContext(EOnlineUserOnlineContext::PlatformOrDefault);
		}
		else
		{
			Request->CurrentContext = ResolveOnlineContext(Request->DesiredContext);
		}
	}

	ELoginStatusType CurrentStatus = GetLocalUserLoginStatus(PlatformUser, Request->CurrentContext);
	FUniqueNetIdRepl CurrentId = GetLocalUserNetId(PlatformUser, Request->CurrentContext);
	FOnlineContextCache* System = GetContextCache(Request->CurrentContext);

	if (!ensure(System))
	{
		return;
	}

	// Starting a new request
	if (Request->OverallLoginState == EOnlineUserAsyncTaskState::NotStarted)
	{
		Request->OverallLoginState = EOnlineUserAsyncTaskState::InProgress;
	}

	bool bHasRequiredStatus = (CurrentStatus == ELoginStatusType::LoggedIn);
	if (Request->DesiredPrivilege == EOnlineUserPrivilege::CanPlay)
	{
		// If this is not an online required login, allow local profile to count as fully logged in
		bHasRequiredStatus |= (CurrentStatus == ELoginStatusType::UsingLocalProfile);
	}

	// Check for overall success
	if (bHasRequiredStatus && CurrentId.IsValid())
	{
		// Stall if we're waiting for the login UI to close
		if (Request->LoginUIState == EOnlineUserAsyncTaskState::InProgress)
		{
			return;
		}

		Request->OverallLoginState = EOnlineUserAsyncTaskState::Done;
	}
	else
	{
		// Try using platform auth to login
		if (Request->TransferPlatformAuthState == EOnlineUserAsyncTaskState::NotStarted)
		{
			Request->TransferPlatformAuthState = EOnlineUserAsyncTaskState::InProgress;

			if (TransferPlatformAuth(System, Request, PlatformUser))
			{
				return;
			}
			// We didn't start a login attempt, so set failure
			Request->TransferPlatformAuthState = EOnlineUserAsyncTaskState::Failed;
		}

		// Next check AutoLogin
		if (Request->AutoLoginState == EOnlineUserAsyncTaskState::NotStarted)
		{
			if (Request->TransferPlatformAuthState == EOnlineUserAsyncTaskState::Done || Request->TransferPlatformAuthState == EOnlineUserAsyncTaskState::Failed)
			{
				Request->AutoLoginState = EOnlineUserAsyncTaskState::InProgress;

				// Try an auto login with default credentials, this will work on many platforms
				if (AutoLogin(System, Request, PlatformUser))
				{
					return;
				}
				// We didn't start an autologin attempt, so set failure
				Request->AutoLoginState = EOnlineUserAsyncTaskState::Failed;
			}
		}

		// Next check login UI
		if (Request->LoginUIState == EOnlineUserAsyncTaskState::NotStarted)
		{
			if ((Request->TransferPlatformAuthState == EOnlineUserAsyncTaskState::Done || Request->TransferPlatformAuthState == EOnlineUserAsyncTaskState::Failed)
				&& (Request->AutoLoginState == EOnlineUserAsyncTaskState::Done || Request->AutoLoginState == EOnlineUserAsyncTaskState::Failed))
			{
				Request->LoginUIState = EOnlineUserAsyncTaskState::InProgress;

				if (ShowLoginUI(System, Request, PlatformUser))
				{
					return;
				}
				// We didn't show a UI, so set failure
				Request->LoginUIState = EOnlineUserAsyncTaskState::Failed;
			}
		}
	}

	// Check for overall failure
	if (Request->LoginUIState == EOnlineUserAsyncTaskState::Failed &&
		Request->AutoLoginState == EOnlineUserAsyncTaskState::Failed &&
		Request->TransferPlatformAuthState == EOnlineUserAsyncTaskState::Failed)
	{
		Request->OverallLoginState = EOnlineUserAsyncTaskState::Failed;
	}
	else if (Request->OverallLoginState == EOnlineUserAsyncTaskState::InProgress &&
		Request->LoginUIState != EOnlineUserAsyncTaskState::InProgress &&
		Request->AutoLoginState != EOnlineUserAsyncTaskState::InProgress &&
		Request->TransferPlatformAuthState != EOnlineUserAsyncTaskState::InProgress)
	{
		// If none of the substates are still in progress but we haven't successfully logged in, mark this as a failure to avoid stalling forever
		Request->OverallLoginState = EOnlineUserAsyncTaskState::Failed;
	}

	if (Request->OverallLoginState == EOnlineUserAsyncTaskState::Done)
	{
		// Do the permissions check if needed
		if (Request->PrivilegeCheckState == EOnlineUserAsyncTaskState::NotStarted)
		{
			Request->PrivilegeCheckState = EOnlineUserAsyncTaskState::InProgress;

			EOnlineUserPrivilegeResult CachedResult = UserInfo->GetCachedPrivilegeResult(Request->DesiredPrivilege, Request->CurrentContext);
			if (CachedResult == EOnlineUserPrivilegeResult::Available)
			{
				// Use cached success value
				Request->PrivilegeCheckState = EOnlineUserAsyncTaskState::Done;
			}
			else
			{
				if (QueryUserPrivilege(System, Request, PlatformUser))
				{
					return;
				}
				else
				{
#if !ONLINEUSER_OSSV1
					// Temp while OSSv2 gets privileges implemented
					CachedResult = EOnlineUserPrivilegeResult::Available;
					Request->PrivilegeCheckState = EOnlineUserAsyncTaskState::Done;
#endif
				}
			}
		}

		if (Request->PrivilegeCheckState == EOnlineUserAsyncTaskState::Failed)
		{
			// Count a privilege failure as a login failure
			Request->OverallLoginState = EOnlineUserAsyncTaskState::Failed;
		}
		else if (Request->PrivilegeCheckState == EOnlineUserAsyncTaskState::Done)
		{
			// If platform context done but still need to do service context, do that next
			EOnlineUserOnlineContext ResolvedDesiredContext = ResolveOnlineContext(Request->DesiredContext);

			if (Request->OverallLoginState == EOnlineUserAsyncTaskState::Done && Request->CurrentContext != ResolvedDesiredContext)
			{
				Request->CurrentContext = ResolvedDesiredContext;
				Request->OverallLoginState = EOnlineUserAsyncTaskState::NotStarted;
				Request->PrivilegeCheckState = EOnlineUserAsyncTaskState::NotStarted;
				Request->TransferPlatformAuthState = EOnlineUserAsyncTaskState::NotStarted;
				Request->AutoLoginState = EOnlineUserAsyncTaskState::NotStarted;
				Request->LoginUIState = EOnlineUserAsyncTaskState::NotStarted;

				// Reprocess and immediately return
				ProcessLoginRequest(Request);
				return;
			}
		}
	}

	if (Request->PrivilegeCheckState == EOnlineUserAsyncTaskState::InProgress)
	{
		// Stall to wait for it to finish
		return;
	}

	// If done, remove and do callback
	if (Request->OverallLoginState == EOnlineUserAsyncTaskState::Done || Request->OverallLoginState == EOnlineUserAsyncTaskState::Failed)
	{
		// Skip if this already happened in a nested function
		if (ActiveLoginRequests.Contains(Request))
		{
			// Add a generic error if none is set
			if (Request->OverallLoginState == EOnlineUserAsyncTaskState::Failed && !Request->Error.IsSet())
			{
#if ONLINEUSER_OSSV1
				Request->Error = FOnlineError(NSLOCTEXT("OnlineUser", "FailedToRequest", "Failed to Request Login"));
#else
				Request->Error = UE::Online::Errors::RequestFailure();
#endif
			}

			// Remove from active array
			ActiveLoginRequests.Remove(Request);

			// Execute delegate if bound
			Request->Delegate.ExecuteIfBound(UserInfo, CurrentStatus, CurrentId, Request->Error, Request->DesiredContext);
		}
	}
}

bool UOnlineUserSubsystem::TransferPlatformAuth(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
#if ONLINEUSER_OSSV1
	// v1版本不支持
	return false;
#else
	return TransferPlatformAuthOSSv2(System, Request, PlatformUser);
#endif
}

bool UOnlineUserSubsystem::AutoLogin(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	UE_LOG(LogOnlineUser, Log, TEXT("Player AutoLogin requested - UserIdx:%d, Privilege:%d, Context:%d"),
	       PlatformUser.GetInternalId(),
	       (int32)Request->DesiredPrivilege,
	       (int32)Request->DesiredContext);

#if ONLINEUSER_OSSV1
	return AutoLoginOSSv1(System, Request, PlatformUser);
#endif

#if !ONLINEUSER_OSSV1
	return AutoLoginOSSv2(System, Request, PlatformUser);
#endif
}

bool UOnlineUserSubsystem::ShowLoginUI(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	UE_LOG(LogOnlineUser, Log, TEXT("Player LoginUI requested - UserIdx:%d, Privilege:%d, Context:%d"),
	       PlatformUser.GetInternalId(),
	       (int32)Request->DesiredPrivilege,
	       (int32)Request->DesiredContext);

#if ONLINEUSER_OSSV1
	return ShowLoginUIOSSv1(System, Request, PlatformUser);
#endif

#if !ONLINEUSER_OSSV1
	return ShowLoginUIOSSv2(System, Request, PlatformUser);
#endif
}

bool UOnlineUserSubsystem::QueryUserPrivilege(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
#if ONLINEUSER_OSSV1
	return QueryUserPrivilegeOSSv1(System, Request, PlatformUser);
#endif

#if !ONLINEUSER_OSSV1
	return QueryUserPrivilegeOSSv2(System, Request, PlatformUser);
#endif
}


void UOnlineUserSubsystem::HandleInputDeviceConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId)
{
	FString InputDeviceIDString = FString::Printf(TEXT("%d"), InputDeviceId.GetId());
	const bool bIsConnected = NewConnectionState == EInputDeviceConnectionState::Connected;
	UE_LOG(LogOnlineUser, Log, TEXT("Controller connection changed - UserIdx:%s, UserID:%s, Connected:%d"), *InputDeviceIDString, *PlatformUserIdToString(PlatformUserId),
	       bIsConnected ? 1 : 0);

	// TODO Implement for platforms that support this
}

static inline FText GetErrorText(const FOnlineErrorType& InOnlineError)
{
#if ONLINEUSER_OSSV1
	return InOnlineError.GetErrorMessage();
#else
	return InOnlineError.GetText();
#endif
}

void UOnlineUserSubsystem::HandleLoginForUserInitialize(const UOnlineUserInfo* UserInfo, ELoginStatusType NewStatus, FUniqueNetIdRepl NetId,
                                                        const TOptional<FOnlineErrorType>& InError, EOnlineUserOnlineContext Context, FOnlineUserInitializeParams Params)
{
	UGameInstance* GameInstance = GetGameInstance();
	check(GameInstance);
	FTimerManager& TimerManager = GameInstance->GetTimerManager();
	TOptional<FOnlineErrorType> Error = InError; // Copy so we can reset on handled errors

	UOnlineUserInfo* LocalUserInfo = ModifyInfo(UserInfo);
	UOnlineUserInfo* FirstUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(0));

	if (!ensure(LocalUserInfo && FirstUserInfo))
	{
		return;
	}

	// Check the hard platform/service ids
	RefreshLocalUserInfo(LocalUserInfo);

	FUniqueNetIdRepl FirstPlayerId = FirstUserInfo->GetNetId(EOnlineUserOnlineContext::PlatformOrDefault);

	// Check to see if we should make a guest after a login failure. Some platforms return success but reuse the first player's id, count this as a failure
	if (LocalUserInfo != FirstUserInfo && LocalUserInfo->bCanBeGuest && (NewStatus == ELoginStatusType::NotLoggedIn || NetId == FirstPlayerId))
	{
#if ONLINEUSER_OSSV1
		NetId = (FUniqueNetIdRef)FUniqueNetIdString::Create(FString::Printf(TEXT("GuestPlayer%d"), LocalUserInfo->LocalPlayerIndex), NULL_SUBSYSTEM);
#else
		// TODO:  OSSv2 FUniqueNetIdRepl wrapping FAccountId is in progress
		// TODO:  OSSv2 - How to handle guest accounts?
#endif
		LocalUserInfo->bIsGuest = true;
		NewStatus = ELoginStatusType::UsingLocalProfile;
		Error.Reset();
		UE_LOG(LogOnlineUser, Log, TEXT("HandleLoginForUserInitialize created guest id %s for local player %d"), *NetId.ToString(), LocalUserInfo->LocalPlayerIndex);
	}
	else
	{
		LocalUserInfo->bIsGuest = false;
	}

	ensure(LocalUserInfo->IsDoingLogin());

	if (Error.IsSet())
	{
		FText ErrorText = GetErrorText(InError.GetValue());
		TimerManager.SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UOnlineUserSubsystem::HandleUserInitializeFailed, Params, ErrorText));
		return;
	}

	if (Context == EOnlineUserOnlineContext::Game)
	{
		LocalUserInfo->UpdateCachedNetId(NetId, EOnlineUserOnlineContext::Game);
	}

	ULocalPlayer* CurrentPlayer = GameInstance->GetLocalPlayerByIndex(LocalUserInfo->LocalPlayerIndex);
	if (!CurrentPlayer && Params.bCanCreateNewLocalPlayer)
	{
		FString ErrorString;
		CurrentPlayer = GameInstance->CreateLocalPlayer(LocalUserInfo->PlatformUser, ErrorString, true);

		if (!CurrentPlayer)
		{
			TimerManager.SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UOnlineUserSubsystem::HandleUserInitializeFailed, Params,
			                                                               FText::AsCultureInvariant(ErrorString)));
			return;
		}
		ensure(GameInstance->GetLocalPlayerByIndex(LocalUserInfo->LocalPlayerIndex) == CurrentPlayer);
	}

	// Updates controller and net id if needed
	SetLocalPlayerUserInfo(CurrentPlayer, LocalUserInfo);

	// Set a delayed callback
	TimerManager.SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UOnlineUserSubsystem::HandleUserInitializeSucceeded, Params));
}

void UOnlineUserSubsystem::HandleUserInitializeFailed(FOnlineUserInitializeParams Params, FText Error)
{
	UOnlineUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(Params.LocalPlayerIndex));

	if (!LocalUserInfo)
	{
		// The user info was reset since this was scheduled
		return;
	}

	UE_LOG(LogOnlineUser, Warning, TEXT("TryToInitializeUser %d failed with error %s"), LocalUserInfo->LocalPlayerIndex, *Error.ToString());

	// If state is wrong, abort as we might have gotten canceled
	if (!ensure(LocalUserInfo->IsDoingLogin()))
	{
		return;
	}

	// If initial login failed or we ended up totally logged out, set to complete failure
	ELoginStatusType NewStatus = GetLocalUserLoginStatus(Params.PlatformUser, Params.OnlineContext);
	if (NewStatus == ELoginStatusType::NotLoggedIn || LocalUserInfo->InitializationState == EOnlineUserInitializationState::DoingInitialLogin)
	{
		LocalUserInfo->InitializationState = EOnlineUserInitializationState::FailedtoLogin;
	}
	else
	{
		LocalUserInfo->InitializationState = EOnlineUserInitializationState::LoggedInLocalOnly;
	}

	FText TitleText = NSLOCTEXT("OnlineUser", "LoginFailedTitle", "Login Failure");

	if (!Params.bSuppressLoginErrors)
	{
		SendSystemMessage(OnlineUserTags::OnlineUser_SystemMessage_Error_InitializeLocalPlayerFailed, TitleText, Error);
	}

	// Call callbacks
	Params.OnUserInitializeComplete.ExecuteIfBound(LocalUserInfo, false, Error, Params.RequestedPrivilege, Params.OnlineContext);
	OnUserInitializeComplete.Broadcast(LocalUserInfo, false, Error, Params.RequestedPrivilege, Params.OnlineContext);
}

void UOnlineUserSubsystem::HandleUserInitializeSucceeded(FOnlineUserInitializeParams Params)
{
	UOnlineUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(Params.LocalPlayerIndex));

	if (!LocalUserInfo)
	{
		// The user info was reset since this was scheduled
		return;
	}

	// If state is wrong, abort as we might have gotten cancelled
	if (!ensure(LocalUserInfo->IsDoingLogin()))
	{
		return;
	}

	// Fix up state
	if (Params.RequestedPrivilege == EOnlineUserPrivilege::CanPlayOnline)
	{
		LocalUserInfo->InitializationState = EOnlineUserInitializationState::LoggedInOnline;
	}
	else
	{
		LocalUserInfo->InitializationState = EOnlineUserInitializationState::LoggedInLocalOnly;
	}

	ensure(LocalUserInfo->GetPrivilegeAvailability(Params.RequestedPrivilege) == EOnlineUserAvailability::NowAvailable);

	// Call callbacks
	Params.OnUserInitializeComplete.ExecuteIfBound(LocalUserInfo, true, FText(), Params.RequestedPrivilege, Params.OnlineContext);
	OnUserInitializeComplete.Broadcast(LocalUserInfo, true, FText(), Params.RequestedPrivilege, Params.OnlineContext);
}

bool UOnlineUserSubsystem::OverrideInputKeyForLogin(FInputKeyEventArgs& EventArgs)
{
	int32 NextLocalPlayerIndex = INDEX_NONE;

	const UOnlineUserInfo* MappedUser = GetUserInfoForInputDevice(EventArgs.InputDevice);
	if (EventArgs.Event == IE_Pressed)
	{
		if (MappedUser == nullptr || !MappedUser->IsLoggedIn())
		{
			if (MappedUser)
			{
				NextLocalPlayerIndex = MappedUser->LocalPlayerIndex;
			}
			else
			{
				// Find next player
				for (int32 i = 0; i < MaxNumberOfLocalPlayers; i++)
				{
					if (GetLocalPlayerInitializationState(i) == EOnlineUserInitializationState::Unknown)
					{
						NextLocalPlayerIndex = i;
						break;
					}
				}
			}

			if (NextLocalPlayerIndex != INDEX_NONE)
			{
				if (LoginKeysForAnyUser.Contains(EventArgs.Key))
				{
					// If we're in the middle of logging in just return true to ignore platform-specific input
					if (MappedUser && MappedUser->IsDoingLogin())
					{
						return true;
					}

					// Press start screen
					FOnlineUserInitializeParams NewParams = ParamsForLoginKey;
					NewParams.LocalPlayerIndex = NextLocalPlayerIndex;
					NewParams.PrimaryInputDevice = EventArgs.InputDevice;

					return TryToInitializeUser(NewParams);
				}

				// See if this controller id is mapped
				MappedUser = GetUserInfoForInputDevice(EventArgs.InputDevice);

				if (!MappedUser || MappedUser->LocalPlayerIndex == INDEX_NONE)
				{
					if (LoginKeysForNewUser.Contains(EventArgs.Key))
					{
						// If we're in the middle of logging in just return true to ignore platform-specific input
						if (MappedUser && MappedUser->IsDoingLogin())
						{
							return true;
						}

						// Local multiplayer
						FOnlineUserInitializeParams NewParams = ParamsForLoginKey;
						NewParams.LocalPlayerIndex = NextLocalPlayerIndex;
						NewParams.PrimaryInputDevice = EventArgs.InputDevice;

						return TryToInitializeUser(NewParams);
					}
				}
			}
		}
	}

	if (WrappedInputKeyHandler.IsBound())
	{
		return WrappedInputKeyHandler.Execute(EventArgs);
	}

	return false;
}

#if ONLINEUSER_OSSV1
IOnlineSubsystem* UOnlineUserSubsystem::GetOnlineSubsystem(EOnlineUserOnlineContext Context) const
{
	if (const FOnlineContextCache* System = GetContextCache(Context))
		return System->OnlineSubsystem;
	return nullptr;
}

IOnlineIdentity* UOnlineUserSubsystem::GetOnlineIdentity(EOnlineUserOnlineContext Context) const
{
	if (const FOnlineContextCache* System = GetContextCache(Context))
		return System->IdentityInterface.Get();
	return nullptr;
}

FName UOnlineUserSubsystem::GetOnlineSubsystemName(EOnlineUserOnlineContext Context) const
{
	if (IOnlineSubsystem* SubSystem = GetOnlineSubsystem(Context))
		return SubSystem->GetSubsystemName();
	return NAME_None;
}

EOnlineServerConnectionStatus::Type UOnlineUserSubsystem::GetConnectionStatus(EOnlineUserOnlineContext Context) const
{
	if (const FOnlineContextCache* System = GetContextCache(Context))
		return System->CurrentConnectionStatus;
	return EOnlineServerConnectionStatus::ServiceUnavailable;
}

EOnlineUserPrivilege UOnlineUserSubsystem::ConvertOSSPrivilege(EUserPrivileges::Type Privilege) const
{
	switch (Privilege)
	{
	case EUserPrivileges::CanPlay:
		return EOnlineUserPrivilege::CanPlay;
	case EUserPrivileges::CanPlayOnline:
		return EOnlineUserPrivilege::CanPlayOnline;
	case EUserPrivileges::CanCommunicateOnline:
		return EOnlineUserPrivilege::CanCommunicateViaTextOnline; // No good thing to do here, just mapping to text.
	case EUserPrivileges::CanUseUserGeneratedContent:
		return EOnlineUserPrivilege::CanUseUserGeneratedContent;
	case EUserPrivileges::CanUserCrossPlay:
		return EOnlineUserPrivilege::CanUseCrossPlay;
	default:
		return EOnlineUserPrivilege::Invalid_Count;
	}
}

EUserPrivileges::Type UOnlineUserSubsystem::ConvertOSSPrivilege(EOnlineUserPrivilege Privilege) const
{
	switch (Privilege)
	{
	case EOnlineUserPrivilege::CanPlay:
		return EUserPrivileges::CanPlay;
	case EOnlineUserPrivilege::CanPlayOnline:
		return EUserPrivileges::CanPlayOnline;
	case EOnlineUserPrivilege::CanCommunicateViaTextOnline:
	case EOnlineUserPrivilege::CanCommunicateViaVoiceOnline:
		return EUserPrivileges::CanCommunicateOnline;
	case EOnlineUserPrivilege::CanUseUserGeneratedContent:
		return EUserPrivileges::CanUseUserGeneratedContent;
	case EOnlineUserPrivilege::CanUseCrossPlay:
		return EUserPrivileges::CanUserCrossPlay;
	default:
		// No failure type, return CanPlay
		return EUserPrivileges::CanPlay;
	}
}

EOnlineUserPrivilegeResult UOnlineUserSubsystem::ConvertOSSPrivilegeResult(EUserPrivileges::Type Privilege, uint32 Results) const
{
	// The V1 results enum is a bitfield where each platform behaves a bit differently
	if (Results == (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures)
	{
		return EOnlineUserPrivilegeResult::Available;
	}
	if ((Results & (uint32)IOnlineIdentity::EPrivilegeResults::UserNotFound) || (Results & (uint32)IOnlineIdentity::EPrivilegeResults::UserNotLoggedIn))
	{
		return EOnlineUserPrivilegeResult::UserNotLoggedIn;
	}
	if ((Results & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredPatchAvailable) || (Results & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredSystemUpdate))
	{
		return EOnlineUserPrivilegeResult::VersionOutdated;
	}
	if (Results & (uint32)IOnlineIdentity::EPrivilegeResults::AgeRestrictionFailure)
	{
		return EOnlineUserPrivilegeResult::AgeRestricted;
	}
	if (Results & (uint32)IOnlineIdentity::EPrivilegeResults::AccountTypeFailure)
	{
		return EOnlineUserPrivilegeResult::AccountTypeRestricted;
	}
	if (Results & (uint32)IOnlineIdentity::EPrivilegeResults::NetworkConnectionUnavailable)
	{
		return EOnlineUserPrivilegeResult::NetworkConnectionUnavailable;
	}

	// Bucket other account failures together
	uint32 AccountUseFailures = (uint32)IOnlineIdentity::EPrivilegeResults::OnlinePlayRestricted
		| (uint32)IOnlineIdentity::EPrivilegeResults::UGCRestriction
		| (uint32)IOnlineIdentity::EPrivilegeResults::ChatRestriction;

	if (Results & AccountUseFailures)
	{
		return EOnlineUserPrivilegeResult::AccountUseRestricted;
	}

	// If you can't play at all, this is a license failure
	if (Privilege == EUserPrivileges::CanPlay)
	{
		return EOnlineUserPrivilegeResult::LicenseInvalid;
	}

	// Unknown reason
	return EOnlineUserPrivilegeResult::PlatformFailure;
}

void UOnlineUserSubsystem::BindOnlineDelegatesOSSv1()
{
	EOnlineUserOnlineContext ServiceType = ResolveOnlineContext(EOnlineUserOnlineContext::ServiceOrDefault);
	EOnlineUserOnlineContext PlatformType = ResolveOnlineContext(EOnlineUserOnlineContext::PlatformOrDefault);
	FOnlineContextCache* ServiceContext = GetContextCache(ServiceType);
	FOnlineContextCache* PlatformContext = GetContextCache(PlatformType);
	check(ServiceContext && ServiceContext->OnlineSubsystem && PlatformContext && PlatformContext->OnlineSubsystem);
	// Connection delegates need to listen for both systems

	ServiceContext->OnlineSubsystem->AddOnConnectionStatusChangedDelegate_Handle(
		FOnConnectionStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleNetworkConnectionStatusChanged, ServiceType));
	ServiceContext->CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;

	for (int32 PlayerIdx = 0; PlayerIdx < MAX_LOCAL_PLAYERS; PlayerIdx++)
	{
		ServiceContext->IdentityInterface->AddOnLoginStatusChangedDelegate_Handle(
			PlayerIdx, FOnLoginStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleIdentityLoginStatusChanged, ServiceType));
		ServiceContext->IdentityInterface->AddOnLoginCompleteDelegate_Handle(
			PlayerIdx, FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::HandleUserLoginCompleted, ServiceType));
	}

	if (ServiceType != PlatformType)
	{
		PlatformContext->OnlineSubsystem->AddOnConnectionStatusChangedDelegate_Handle(
			FOnConnectionStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleNetworkConnectionStatusChanged, PlatformType));
		PlatformContext->CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;

		for (int32 PlayerIdx = 0; PlayerIdx < MAX_LOCAL_PLAYERS; PlayerIdx++)
		{
			PlatformContext->IdentityInterface->AddOnLoginStatusChangedDelegate_Handle(
				PlayerIdx, FOnLoginStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleIdentityLoginStatusChanged, PlatformType));
			PlatformContext->IdentityInterface->AddOnLoginCompleteDelegate_Handle(
				PlayerIdx, FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::HandleUserLoginCompleted, PlatformType));
		}
	}

	// Hardware change delegates only listen to platform
	PlatformContext->IdentityInterface->AddOnControllerPairingChangedDelegate_Handle(
		FOnControllerPairingChangedDelegate::CreateUObject(this, &ThisClass::HandleControllerPairingChanged));
}

bool UOnlineUserSubsystem::AutoLoginOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	return System->IdentityInterface->AutoLogin(GetPlatformUserIndexForId(PlatformUser));
}

bool UOnlineUserSubsystem::ShowLoginUIOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	IOnlineExternalUIPtr ExternalUI = System->OnlineSubsystem->GetExternalUIInterface();
	if (ExternalUI.IsValid())
	{
		// TODO Unclear which flags should be set
		return ExternalUI->ShowLoginUI(GetPlatformUserIndexForId(PlatformUser), false, false,
		                               FOnLoginUIClosedDelegate::CreateUObject(this, &ThisClass::HandleOnLoginUIClosed, Request->CurrentContext));
	}
	return false;
}

bool UOnlineUserSubsystem::QueryUserPrivilegeOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	// Start query on unknown or failure
	EUserPrivileges::Type OSSPrivilege = ConvertOSSPrivilege(Request->DesiredPrivilege);

	FUniqueNetIdRepl CurrentId = GetLocalUserNetId(PlatformUser, Request->CurrentContext);
	check(CurrentId.IsValid());
	IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate Delegate = IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateUObject(
		this, &UOnlineUserSubsystem::HandleCheckPrivilegesComplete, Request->DesiredPrivilege, Request->UserInfo, Request->CurrentContext);
	System->IdentityInterface->GetUserPrivilege(*CurrentId, OSSPrivilege, Delegate);

	// This may immediately succeed and reenter this function, so we have to return
	return true;
}

void UOnlineUserSubsystem::HandleIdentityLoginStatusChanged(int32 PlatformUserIndex, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& NewId,
                                                            EOnlineUserOnlineContext Context)
{
	UE_LOG(LogOnlineUser, Log, TEXT("Player login status changed - System:%s, UserIdx:%d, OldStatus:%s, NewStatus:%s, NewId:%s"),
	       *GetOnlineSubsystemName(Context).ToString(),
	       PlatformUserIndex,
	       ELoginStatus::ToString(OldStatus),
	       ELoginStatus::ToString(NewStatus),
	       *NewId.ToString());

	if (NewStatus == ELoginStatus::NotLoggedIn && OldStatus != ELoginStatus::NotLoggedIn)
	{
		FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);
		LogOutLocalUser(PlatformUser);
	}
}

void UOnlineUserSubsystem::HandleUserLoginCompleted(int32 PlatformUserIndex, bool bWasSuccessful, const FUniqueNetId& NetId, const FString& ErrorString,
                                                    EOnlineUserOnlineContext Context)
{
	FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);
	ELoginStatusType NewStatus = GetLocalUserLoginStatus(PlatformUser, Context);
	FUniqueNetIdRepl NewId = FUniqueNetIdRepl(NetId);
	UE_LOG(LogOnlineUser, Log, TEXT("Player login Completed - System:%s, UserIdx:%d, Successful:%d, NewStatus:%s, NewId:%s, ErrorIfAny:%s"),
	       *GetOnlineSubsystemName(Context).ToString(),
	       PlatformUserIndex,
	       (int32)bWasSuccessful,
	       ELoginStatus::ToString(NewStatus),
	       *NewId.ToString(),
	       *ErrorString);

	// Update any waiting login requests
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		UOnlineUserInfo* UserInfo = Request->UserInfo.Get();

		if (!UserInfo)
		{
			// User is gone, just delete this request
			ActiveLoginRequests.Remove(Request);

			continue;
		}

		if (UserInfo->PlatformUser == PlatformUser && Request->CurrentContext == Context)
		{
			// On some platforms this gets called from the login UI with a failure
			if (Request->AutoLoginState == EOnlineUserAsyncTaskState::InProgress)
			{
				Request->AutoLoginState = bWasSuccessful ? EOnlineUserAsyncTaskState::Done : EOnlineUserAsyncTaskState::Failed;
			}

			if (!bWasSuccessful)
			{
				Request->Error = FOnlineError(FText::FromString(ErrorString));
			}

			ProcessLoginRequest(Request);
		}
	}
}

void UOnlineUserSubsystem::HandleControllerPairingChanged(int32 PlatformUserIndex, FControllerPairingChangedUserInfo PreviousUser, FControllerPairingChangedUserInfo NewUser)
{
	UE_LOG(LogOnlineUser, Log, TEXT("Player controller pairing changed - UserIdx:%d, PreviousUser:%s, NewUser:%s"),
	       PlatformUserIndex,
	       *ToDebugString(PreviousUser),
	       *ToDebugString(NewUser));

	UGameInstance* GameInstance = GetGameInstance();
	FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);
	ULocalPlayer* ControlledLocalPlayer = GameInstance->FindLocalPlayerFromPlatformUserId(PlatformUser);
	ULocalPlayer* NewLocalPlayer = GameInstance->FindLocalPlayerFromUniqueNetId(NewUser.User);
	const UOnlineUserInfo* NewUserInfo = GetUserInfoForUniqueNetId(FUniqueNetIdRepl(NewUser.User));
	const UOnlineUserInfo* PreviousUserInfo = GetUserInfoForUniqueNetId(FUniqueNetIdRepl(PreviousUser.User));

	// See if we think this is already bound to an existing player	
	if (PreviousUser.ControllersRemaining == 0 && PreviousUserInfo && PreviousUserInfo != NewUserInfo)
	{
		// This means that the user deliberately logged out using a platform interface
		if (IsRealPlatformUser(PlatformUser))
		{
			LogOutLocalUser(PlatformUser);
		}
	}

	if (ControlledLocalPlayer && ControlledLocalPlayer != NewLocalPlayer)
	{
		// TODO Currently the platforms that call this delegate do not really handle swapping controller IDs
		// SetLocalPlayerUserIndex(ControlledLocalPlayer, -1);
	}
}

void UOnlineUserSubsystem::HandleNetworkConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus,
                                                                EOnlineServerConnectionStatus::Type ConnectionStatus, EOnlineUserOnlineContext Context)
{
	UE_LOG(LogOnlineUser, Log, TEXT("HandleNetworkConnectionStatusChanged(ServiceName: %s, LastStatus: %s, ConnectionStatus: %s)"),
	       *ServiceName,
	       EOnlineServerConnectionStatus::ToString(LastConnectionStatus),
	       EOnlineServerConnectionStatus::ToString(ConnectionStatus));

	// Cache old availablity for current users
	TMap<UOnlineUserInfo*, EOnlineUserAvailability> AvailabilityMap;

	for (TPair<int32, UOnlineUserInfo*> Pair : LocalUserInfos)
	{
		AvailabilityMap.Add(Pair.Value, Pair.Value->GetPrivilegeAvailability(EOnlineUserPrivilege::CanPlayOnline));
	}

	FOnlineContextCache* System = GetContextCache(Context);
	if (ensure(System))
	{
		// Service name is normally the same as the OSS name, but not necessarily on all platforms
		System->CurrentConnectionStatus = ConnectionStatus;
	}

	for (TPair<UOnlineUserInfo*, EOnlineUserAvailability> Pair : AvailabilityMap)
	{
		// Notify other systems when someone goes online/offline
		HandleChangedAvailability(Pair.Key, EOnlineUserPrivilege::CanPlayOnline, Pair.Value);
	}
}

void UOnlineUserSubsystem::HandleOnLoginUIClosed(TSharedPtr<const FUniqueNetId> LoggedInNetId, const int PlatformUserIndex, const FOnlineError& Error,
                                                 EOnlineUserOnlineContext Context)
{
	FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);

	// Update any waiting login requests
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		UOnlineUserInfo* UserInfo = Request->UserInfo.Get();

		if (!UserInfo)
		{
			// User is gone, just delete this request
			ActiveLoginRequests.Remove(Request);

			continue;
		}

		// Look for first user trying to log in on this context
		if (Request->CurrentContext == Context && Request->LoginUIState == EOnlineUserAsyncTaskState::InProgress)
		{
			if (LoggedInNetId.IsValid() && LoggedInNetId->IsValid() && Error.WasSuccessful())
			{
				// The platform user id that actually logged in may not be the same one who requested the UI,
				// so swap it if the returned id is actually valid
				if (UserInfo->PlatformUser != PlatformUser && PlatformUser != PLATFORMUSERID_NONE)
				{
					UserInfo->PlatformUser = PlatformUser;
				}

				Request->LoginUIState = EOnlineUserAsyncTaskState::Done;
				Request->Error.Reset();
			}
			else
			{
				Request->LoginUIState = EOnlineUserAsyncTaskState::Failed;
				Request->Error = Error;
			}

			ProcessLoginRequest(Request);
		}
	}
}

void UOnlineUserSubsystem::HandleCheckPrivilegesComplete(const FUniqueNetId& UserId, EUserPrivileges::Type UserPrivilege, uint32 PrivilegeResults,
                                                         EOnlineUserPrivilege RequestedPrivilege, TWeakObjectPtr<UOnlineUserInfo> OnlineUserInfo, EOnlineUserOnlineContext Context)
{
	// Only handle if user still exists
	UOnlineUserInfo* UserInfo = OnlineUserInfo.Get();

	if (!UserInfo)
	{
		return;
	}

	EOnlineUserPrivilegeResult UserResult = ConvertOSSPrivilegeResult(UserPrivilege, PrivilegeResults);

	// Update the user cached value
	UpdateUserPrivilegeResult(UserInfo, RequestedPrivilege, UserResult, Context);

	FOnlineContextCache* ContextCache = GetContextCache(Context);
	check(ContextCache);

	// If this returns disconnected, update the connection status
	if (UserResult == EOnlineUserPrivilegeResult::NetworkConnectionUnavailable)
	{
		ContextCache->CurrentConnectionStatus = EOnlineServerConnectionStatus::NoNetworkConnection;
	}
	else if (UserResult == EOnlineUserPrivilegeResult::Available && RequestedPrivilege == EOnlineUserPrivilege::CanPlayOnline)
	{
		if (ContextCache->CurrentConnectionStatus == EOnlineServerConnectionStatus::NoNetworkConnection)
		{
			ContextCache->CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;
		}
	}

	// See if a login request is waiting on this
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		if (Request->UserInfo.Get() == UserInfo && Request->CurrentContext == Context && Request->DesiredPrivilege == RequestedPrivilege && Request->PrivilegeCheckState ==
			EOnlineUserAsyncTaskState::InProgress)
		{
			if (UserResult == EOnlineUserPrivilegeResult::Available)
			{
				Request->PrivilegeCheckState = EOnlineUserAsyncTaskState::Done;
			}
			else
			{
				Request->PrivilegeCheckState = EOnlineUserAsyncTaskState::Failed;

				// Forms strings in english like "(The user is not allowed) to (play the game)"
				Request->Error = FOnlineError(FText::Format(
					NSLOCTEXT("OnlineUser", "PrivilegeFailureFormat", "{0} to {1}"), GetPrivilegeResultDescription(UserResult), GetPrivilegeDescription(RequestedPrivilege)));
			}

			ProcessLoginRequest(Request);
		}
	}
}
#endif

#if !ONLINEUSER_OSSV1
UE::Online::EOnlineServices UOnlineUserSubsystem::GetOnlineServicesProvider(EOnlineUserOnlineContext Context) const
{
	if (const FOnlineContextCache* System = GetContextCache(Context))
		return System->OnlineServices->GetServicesProvider();

	return UE::Online::EOnlineServices::None;
}

UE::Online::IAuthPtr UOnlineUserSubsystem::GetOnlineAuth(EOnlineUserOnlineContext Context) const
{
	if (const FOnlineContextCache* System = GetContextCache(Context))
		return System->AuthService;

	return nullptr;
}

UE::Online::EOnlineServicesConnectionStatus UOnlineUserSubsystem::GetConnectionStatus(EOnlineUserOnlineContext Context) const
{
	if (const FOnlineContextCache* System = GetContextCache(Context))
		return System->CurrentConnectionStatus;

	return UE::Online::EOnlineServicesConnectionStatus::NotConnected;
}

EOnlineUserPrivilege UOnlineUserSubsystem::ConvertOnlineServicesPrivilege(UE::Online::EUserPrivileges Privilege) const
{
	switch (Privilege)
	{
	case EUserPrivileges::CanPlay:
		return EOnlineUserPrivilege::CanPlay;
	case EUserPrivileges::CanPlayOnline:
		return EOnlineUserPrivilege::CanPlayOnline;
	case EUserPrivileges::CanCommunicateViaTextOnline:
		return EOnlineUserPrivilege::CanCommunicateViaTextOnline;
	case EUserPrivileges::CanCommunicateViaVoiceOnline:
		return EOnlineUserPrivilege::CanCommunicateViaVoiceOnline;
	case EUserPrivileges::CanUseUserGeneratedContent:
		return EOnlineUserPrivilege::CanUseUserGeneratedContent;
	case EUserPrivileges::CanCrossPlay:
		return EOnlineUserPrivilege::CanUseCrossPlay;
	default:
		return EOnlineUserPrivilege::Invalid_Count;
	}
}

UE::Online::EUserPrivileges UOnlineUserSubsystem::ConvertOnlineServicesPrivilege(EOnlineUserPrivilege Privilege) const
{
	switch (Privilege)
	{
	case EOnlineUserPrivilege::CanPlay:
		return EUserPrivileges::CanPlay;
	case EOnlineUserPrivilege::CanPlayOnline:
		return EUserPrivileges::CanPlayOnline;
	case EOnlineUserPrivilege::CanCommunicateViaTextOnline:
		return EUserPrivileges::CanCommunicateViaTextOnline;
	case EOnlineUserPrivilege::CanCommunicateViaVoiceOnline:
		return EUserPrivileges::CanCommunicateViaVoiceOnline;
	case EOnlineUserPrivilege::CanUseUserGeneratedContent:
		return EUserPrivileges::CanUseUserGeneratedContent;
	case EOnlineUserPrivilege::CanUseCrossPlay:
		return EUserPrivileges::CanCrossPlay;
	default:
		// 无故障类型，返回CanPlay
		return EUserPrivileges::CanPlay;
	}
}

EOnlineUserPrivilegeResult UOnlineUserSubsystem::ConvertOnlineServicesPrivilegeResult(UE::Online::EUserPrivileges Privilege, UE::Online::EPrivilegeResults Results) const
{
	// V1结果枚举是一个位字段，其中每个平台的行为略有不同
	if (Results == EPrivilegeResults::NoFailures)
	{
		return EOnlineUserPrivilegeResult::Available;
	}
	if (EnumHasAnyFlags(Results, EPrivilegeResults::UserNotFound | EPrivilegeResults::UserNotLoggedIn))
	{
		return EOnlineUserPrivilegeResult::UserNotLoggedIn;
	}
	if (EnumHasAnyFlags(Results, EPrivilegeResults::RequiredPatchAvailable | EPrivilegeResults::RequiredSystemUpdate))
	{
		return EOnlineUserPrivilegeResult::VersionOutdated;
	}
	if (EnumHasAnyFlags(Results, EPrivilegeResults::AgeRestrictionFailure))
	{
		return EOnlineUserPrivilegeResult::AgeRestricted;
	}
	if (EnumHasAnyFlags(Results, EPrivilegeResults::AccountTypeFailure))
	{
		return EOnlineUserPrivilegeResult::AccountTypeRestricted;
	}
	if (EnumHasAnyFlags(Results, EPrivilegeResults::NetworkConnectionUnavailable))
	{
		return EOnlineUserPrivilegeResult::NetworkConnectionUnavailable;
	}

	// Bucket other account failures together
	const EPrivilegeResults AccountUseFailures = EPrivilegeResults::OnlinePlayRestricted
		| EPrivilegeResults::UGCRestriction
		| EPrivilegeResults::ChatRestriction;

	if (EnumHasAnyFlags(Results, AccountUseFailures))
	{
		return EOnlineUserPrivilegeResult::AccountUseRestricted;
	}

	// If you can't play at all, this is a license failure
	if (Privilege == EUserPrivileges::CanPlay)
	{
		return EOnlineUserPrivilegeResult::LicenseInvalid;
	}

	// Unknown reason
	return EOnlineUserPrivilegeResult::PlatformFailure;
}

void UOnlineUserSubsystem::BindOnlineDelegatesOSSv2()
{
	EOnlineUserOnlineContext ServiceType = ResolveOnlineContext(EOnlineUserOnlineContext::ServiceOrDefault);
	EOnlineUserOnlineContext PlatformType = ResolveOnlineContext(EOnlineUserOnlineContext::PlatformOrDefault);
	FOnlineContextCache* ServiceContext = GetContextCache(ServiceType);
	FOnlineContextCache* PlatformContext = GetContextCache(PlatformType);
	check(ServiceContext && ServiceContext->OnlineServices && PlatformContext && PlatformContext->OnlineServices);

	ServiceContext->LoginStatusChangedHandle = ServiceContext->AuthService->OnLoginStatusChanged().Add(this, &ThisClass::HandleAuthLoginStatusChanged, ServiceType);
	if (IConnectivityPtr ConnectivityInterface = ServiceContext->OnlineServices->GetConnectivityInterface())
	{
		ServiceContext->ConnectionStatusChangedHandle = ConnectivityInterface->OnConnectionStatusChanged().Add(this, &ThisClass::HandleNetworkConnectionStatusChanged, ServiceType);
	}
	CacheConnectionStatus(ServiceType);

	if (ServiceType != PlatformType)
	{
		PlatformContext->LoginStatusChangedHandle = PlatformContext->AuthService->OnLoginStatusChanged().Add(this, &ThisClass::HandleAuthLoginStatusChanged, PlatformType);
		if (IConnectivityPtr ConnectivityInterface = PlatformContext->OnlineServices->GetConnectivityInterface())
		{
			PlatformContext->ConnectionStatusChangedHandle = ConnectivityInterface->OnConnectionStatusChanged().Add(
				this, &ThisClass::HandleNetworkConnectionStatusChanged, PlatformType);
		}
		CacheConnectionStatus(PlatformType);
	}
	// TODO:  Controller Pairing Changed - move out of OSS and listen to CoreDelegate directly?
}

void UOnlineUserSubsystem::CacheConnectionStatus(EOnlineUserOnlineContext Context)
{
	FOnlineContextCache* ContextCache = GetContextCache(Context);
	check(ContextCache);

	EOnlineServicesConnectionStatus ConnectionStatus = EOnlineServicesConnectionStatus::NotConnected;
	if (IConnectivityPtr ConnectivityInterface = ContextCache->OnlineServices->GetConnectivityInterface())
	{
		const TOnlineResult<FGetConnectionStatus> Result = ConnectivityInterface->GetConnectionStatus(FGetConnectionStatus::Params());
		if (Result.IsOk())
		{
			ConnectionStatus = Result.GetOkValue().Status;
		}
	}
	else
	{
		ConnectionStatus = EOnlineServicesConnectionStatus::Connected;
	}

	UE::Online::FConnectionStatusChanged EventParams;
	EventParams.PreviousStatus = ContextCache->CurrentConnectionStatus;
	EventParams.CurrentStatus = ConnectionStatus;
	HandleNetworkConnectionStatusChanged(EventParams, Context);
}

bool UOnlineUserSubsystem::TransferPlatformAuthOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	IAuthPtr PlatformAuthInterface = GetOnlineAuth(EOnlineUserOnlineContext::Platform);
	if (Request->CurrentContext != EOnlineUserOnlineContext::Platform
		&& PlatformAuthInterface)
	{
		FAuthQueryExternalAuthToken::Params Params;
		Params.LocalAccountId = GetLocalUserNetId(PlatformUser, EOnlineUserOnlineContext::Platform).GetV2();

		PlatformAuthInterface->QueryExternalAuthToken(MoveTemp(Params))
		                     .OnComplete(this, [this, Request](const TOnlineResult<FAuthQueryExternalAuthToken>& Result)
		                     {
			                     UOnlineUserInfo* UserInfo = Request->UserInfo.Get();
			                     if (!UserInfo)
			                     {
				                     // User is gone, just delete this request
				                     ActiveLoginRequests.Remove(Request);
				                     return;
			                     }

			                     if (Result.IsOk())
			                     {
				                     const FAuthQueryExternalAuthToken::Result& GenerateAuthTokenResult = Result.GetOkValue();
				                     FAuthLogin::Params Params;
				                     Params.PlatformUserId = UserInfo->GetPlatformUserId();
				                     Params.CredentialsType = LoginCredentialsType::ExternalAuth;
				                     Params.CredentialsToken.Emplace<FExternalAuthToken>(GenerateAuthTokenResult.ExternalAuthToken);

				                     IAuthPtr PrimaryAuthInterface = GetOnlineAuth(Request->CurrentContext);
				                     PrimaryAuthInterface->Login(MoveTemp(Params))
				                                         .OnComplete(this, [this, Request](const TOnlineResult<FAuthLogin>& Result)
				                                         {
					                                         UOnlineUserInfo* UserInfo = Request->UserInfo.Get();
					                                         if (!UserInfo)
					                                         {
						                                         // User is gone, just delete this request
						                                         ActiveLoginRequests.Remove(Request);
						                                         return;
					                                         }

					                                         if (Result.IsOk())
					                                         {
						                                         Request->TransferPlatformAuthState = EOnlineUserAsyncTaskState::Done;
						                                         Request->Error.Reset();
					                                         }
					                                         else
					                                         {
						                                         Request->TransferPlatformAuthState = EOnlineUserAsyncTaskState::Failed;
						                                         Request->Error = Result.GetErrorValue();
					                                         }
					                                         ProcessLoginRequest(Request);
				                                         });
			                     }
			                     else
			                     {
				                     Request->TransferPlatformAuthState = EOnlineUserAsyncTaskState::Failed;
				                     Request->Error = Result.GetErrorValue();
				                     ProcessLoginRequest(Request);
			                     }
		                     });
		return true;
	}
	return false;
}

bool UOnlineUserSubsystem::AutoLoginOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	FAuthLogin::Params LoginParameters;
	LoginParameters.PlatformUserId = PlatformUser;
	LoginParameters.CredentialsType = LoginCredentialsType::Auto;
	// Leave other LoginParameters as default to allow the online service to determine how to try to automatically log in the user
	TOnlineAsyncOpHandle<FAuthLogin> LoginHandle = System->AuthService->Login(MoveTemp(LoginParameters));
	LoginHandle.OnComplete(this, &ThisClass::HandleUserLoginCompletedV2, PlatformUser, Request->CurrentContext);
	return true;
}

bool UOnlineUserSubsystem::ShowLoginUIOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	IExternalUIPtr ExternalUI = System->OnlineServices->GetExternalUIInterface();
	if (ExternalUI.IsValid())
	{
		FExternalUIShowLoginUI::Params ShowLoginUIParameters;
		ShowLoginUIParameters.PlatformUserId = PlatformUser;
		TOnlineAsyncOpHandle<FExternalUIShowLoginUI> LoginHandle = ExternalUI->ShowLoginUI(MoveTemp(ShowLoginUIParameters));
		LoginHandle.OnComplete(this, &ThisClass::HandleOnLoginUIClosedV2, PlatformUser, Request->CurrentContext);
		return true;
	}
	return false;
}

bool UOnlineUserSubsystem::QueryUserPrivilegeOSSv2(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	UOnlineUserInfo* UserInfo = Request->UserInfo.Get();

	if (IPrivilegesPtr PrivilegesInterface = System->OnlineServices->GetPrivilegesInterface())
	{
		const EUserPrivileges DesiredPrivilege = ConvertOnlineServicesPrivilege(Request->DesiredPrivilege);

		FQueryUserPrivilege::Params Params;
		Params.LocalAccountId = GetLocalUserNetId(PlatformUser, Request->CurrentContext).GetV2();
		Params.Privilege = DesiredPrivilege;
		TOnlineAsyncOpHandle<FQueryUserPrivilege> QueryHandle = PrivilegesInterface->QueryUserPrivilege(MoveTemp(Params));
		QueryHandle.OnComplete(this, &ThisClass::HandleCheckPrivilegesComplete, Request->UserInfo, DesiredPrivilege, Request->CurrentContext);
		return true;
	}
	else
	{
		UpdateUserPrivilegeResult(UserInfo, Request->DesiredPrivilege, EOnlineUserPrivilegeResult::Available, Request->CurrentContext);
	}
	return false;
}

TSharedPtr<UE::Online::FAccountInfo> UOnlineUserSubsystem::GetOnlineServiceAccountInfo(UE::Online::IAuthPtr AuthService, FPlatformUserId InUserId) const
{
	TSharedPtr<FAccountInfo> AccountInfo;
	FAuthGetLocalOnlineUserByPlatformUserId::Params GetAccountParams = {InUserId};
	TOnlineResult<FAuthGetLocalOnlineUserByPlatformUserId> GetAccountResult = AuthService->GetLocalOnlineUserByPlatformUserId(MoveTemp(GetAccountParams));
	if (GetAccountResult.IsOk())
	{
		AccountInfo = GetAccountResult.GetOkValue().AccountInfo;
	}
	return AccountInfo;
}

void UOnlineUserSubsystem::HandleAuthLoginStatusChanged(const UE::Online::FAuthLoginStatusChanged& EventParameters, EOnlineUserOnlineContext Context)
{
	UE_LOG(LogOnlineUser, Log, TEXT("Player login status changed - System:%d, UserId:%s, NewStatus:%s"),
	       (int)Context,
	       *ToLogString(EventParameters.AccountInfo->AccountId),
	       LexToString(EventParameters.LoginStatus));
}

void UOnlineUserSubsystem::HandleUserLoginCompletedV2(const UE::Online::TOnlineResult<UE::Online::FAuthLogin>& Result, FPlatformUserId PlatformUser,
                                                      EOnlineUserOnlineContext Context)
{
	const bool bWasSuccessful = Result.IsOk();
	FAccountId NewId;
	if (bWasSuccessful)
	{
		NewId = Result.GetOkValue().AccountInfo->AccountId;
	}

	ELoginStatusType NewStatus = GetLocalUserLoginStatus(PlatformUser, Context);
	UE_LOG(LogOnlineUser, Log, TEXT("Player login Completed - System:%d, UserIdx:%d, Successful:%d, NewId:%s, ErrorIfAny:%s"),
	       (int32)Context,
	       PlatformUser.GetInternalId(),
	       (int32)Result.IsOk(),
	       *ToLogString(NewId),
	       Result.IsError() ? *Result.GetErrorValue().GetLogString() : TEXT(""));

	// Update any waiting login requests
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		UOnlineUserInfo* UserInfo = Request->UserInfo.Get();

		if (!UserInfo)
		{
			// User is gone, just delete this request
			ActiveLoginRequests.Remove(Request);

			continue;
		}

		if (UserInfo->PlatformUser == PlatformUser && Request->CurrentContext == Context)
		{
			// On some platforms this gets called from the login UI with a failure
			if (Request->AutoLoginState == EOnlineUserAsyncTaskState::InProgress)
			{
				Request->AutoLoginState = bWasSuccessful ? EOnlineUserAsyncTaskState::Done : EOnlineUserAsyncTaskState::Failed;
			}

			if (bWasSuccessful)
			{
				Request->Error.Reset();
			}
			else
			{
				Request->Error = Result.GetErrorValue();
			}

			ProcessLoginRequest(Request);
		}
	}
}

void UOnlineUserSubsystem::HandleOnLoginUIClosedV2(const UE::Online::TOnlineResult<UE::Online::FExternalUIShowLoginUI>& Result, FPlatformUserId PlatformUser,
                                                   EOnlineUserOnlineContext Context)
{
	// Update any waiting login requests
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		UOnlineUserInfo* UserInfo = Request->UserInfo.Get();

		if (!UserInfo)
		{
			// User is gone, just delete this request
			ActiveLoginRequests.Remove(Request);

			continue;
		}

		// Look for first user trying to log in on this context
		if (Request->CurrentContext == Context && Request->LoginUIState == EOnlineUserAsyncTaskState::InProgress)
		{
			if (Result.IsOk())
			{
				// The platform user id that actually logged in may not be the same one who requested the UI,
				// so swap it if the returned id is actually valid
				if (UserInfo->PlatformUser != PlatformUser && PlatformUser != PLATFORMUSERID_NONE)
				{
					UserInfo->PlatformUser = PlatformUser;
				}

				Request->LoginUIState = EOnlineUserAsyncTaskState::Done;
				Request->Error.Reset();
			}
			else
			{
				Request->LoginUIState = EOnlineUserAsyncTaskState::Failed;
				Request->Error = Result.GetErrorValue();
			}

			ProcessLoginRequest(Request);
		}
	}
}

void UOnlineUserSubsystem::HandleNetworkConnectionStatusChanged(const UE::Online::FConnectionStatusChanged& EventParameters, EOnlineUserOnlineContext Context)
{
	UE_LOG(LogOnlineUser, Log, TEXT("HandleNetworkConnectionStatusChanged(Context:%d, ServiceName:%s, OldStatus:%s, NewStatus:%s)"),
	       (int)Context,
	       *EventParameters.ServiceName,
	       LexToString(EventParameters.PreviousStatus),
	       LexToString(EventParameters.CurrentStatus));

	// Cache old availablity for current users
	TMap<UOnlineUserInfo*, EOnlineUserAvailability> AvailabilityMap;

	for (TPair<int32, UOnlineUserInfo*> Pair : LocalUserInfos)
	{
		AvailabilityMap.Add(Pair.Value, Pair.Value->GetPrivilegeAvailability(EOnlineUserPrivilege::CanPlayOnline));
	}

	FOnlineContextCache* System = GetContextCache(Context);
	if (ensure(System))
	{
		// Service name is normally the same as the OSS name, but not necessarily on all platforms
		System->CurrentConnectionStatus = EventParameters.CurrentStatus;
	}

	for (TPair<UOnlineUserInfo*, EOnlineUserAvailability> Pair : AvailabilityMap)
	{
		// Notify other systems when someone goes online/offline
		HandleChangedAvailability(Pair.Key, EOnlineUserPrivilege::CanPlayOnline, Pair.Value);
	}
}

void UOnlineUserSubsystem::HandleCheckPrivilegesComplete(const UE::Online::TOnlineResult<UE::Online::FQueryUserPrivilege>& Result, TWeakObjectPtr<UOnlineUserInfo> OnlineUserInfo,
                                                         UE::Online::EUserPrivileges DesiredPrivilege, EOnlineUserOnlineContext Context)
{
	// Only handle if user still exists
	UOnlineUserInfo* UserInfo = OnlineUserInfo.Get();
	if (!UserInfo)
	{
		return;
	}

	EOnlineUserPrivilege UserPrivilege = ConvertOnlineServicesPrivilege(DesiredPrivilege);
	EOnlineUserPrivilegeResult UserResult = EOnlineUserPrivilegeResult::PlatformFailure;
	if (const FQueryUserPrivilege::Result* OkResult = Result.TryGetOkValue())
	{
		UserResult = ConvertOnlineServicesPrivilegeResult(DesiredPrivilege, OkResult->PrivilegeResult);
	}
	else
	{
		UE_LOG(LogOnlineUser, Warning, TEXT("QueryUserPrivilege failed: %s"), *Result.GetErrorValue().GetLogString());
	}

	// Update the user cached value
	UserInfo->UpdateCachedPrivilegeResult(UserPrivilege, UserResult, Context);

	// See if a login request is waiting on this
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		if (Request->UserInfo.Get() == UserInfo && Request->CurrentContext == Context && Request->DesiredPrivilege == UserPrivilege && Request->PrivilegeCheckState ==
			EOnlineUserAsyncTaskState::InProgress)
		{
			if (UserResult == EOnlineUserPrivilegeResult::Available)
			{
				Request->PrivilegeCheckState = EOnlineUserAsyncTaskState::Done;
			}
			else
			{
				Request->PrivilegeCheckState = EOnlineUserAsyncTaskState::Failed;
				Request->Error = Result.IsError() ? Result.GetErrorValue() : UE::Online::Errors::Unknown();
			}

			ProcessLoginRequest(Request);
		}
	}
}
#endif

#undef UE_API
