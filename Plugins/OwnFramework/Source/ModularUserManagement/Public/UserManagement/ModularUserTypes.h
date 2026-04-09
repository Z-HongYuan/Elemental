// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

// v1版本使用的Include和转发声明
#if MODULARUSER_OSSV1

#include "OnlineSubsystemTypes.h"
class IOnlineSubsystem;
struct FOnlineError;
using FOnlineErrorType = FOnlineError;
using ELoginStatusType = ELoginStatus::Type;
#include "OnlineError.h"

#endif

// v2版本版本使用的Include和转发声明
#if !MODULARUSER_OSSV1

#include "Online/Connectivity.h"
#include "Online/OnlineError.h"
namespace UE::Online
{
	enum class ELoginStatus : uint8;
	enum class EPrivilegeResults : uint32;
	enum class EUserPrivileges : uint8;
	using IAuthPtr = TSharedPtr<class IAuth>;
	using IOnlineServicesPtr = TSharedPtr<class IOnlineServices>;
	template <typename OpType>
	class TOnlineResult;
	struct FAuthLogin;
	struct FConnectionStatusChanged;
	struct FExternalUIShowLoginUI;
	struct FAuthLoginStatusChanged;
	struct FQueryUserPrivilege;
	struct FAccountInfo;
}
using FOnlineErrorType = UE::Online::FOnlineError;
using ELoginStatusType = UE::Online::ELoginStatus;
#include "Online/OnlineErrorDefinitions.h"

#endif

#include "ModularUserTypes.generated.h"


/** 在什么上下文中如何查询联机权限 */
UENUM(BlueprintType)
enum class EModularUserOnlineContext : uint8
{
	/** 从游戏代码调用，这使用默认系统，但具有特殊处理，可以合并来自多个上下文的结果 */
	Game,

	/** 默认的发动机在线系统，它将始终存在，并与服务或平台相同 */
	Default,

	/** 明确请求可能不存在的外部服务 */
	Service,

	/** 先查找外部服务，然后回退到默认值 */
	ServiceOrDefault,

	/**明确要求平台系统，可能不存在*/
	Platform,

	/** 先查找平台系统，然后回退到默认值 */
	PlatformOrDefault,

	/** 无效的系统 */
	Invalid
};

/** 用户初始化状态的枚举 */
UENUM(BlueprintType)
enum class EModularUserInitializationState : uint8
{
	/** 用户尚未启动登录过程 */
	Unknown,

	/** 玩家正在获取本地登录的用户id*/
	DoingInitialLogin,

	/** 玩家正在进行网络登录，他们已经在本地登录 */
	DoingNetworkLogin,

	/** 玩家根本无法登录*/
	FailedtoLogin,


	/** 玩家已登录并可以访问在线功能*/
	LoggedInOnline,

	/** 玩家已在本地登录（无论是访客还是真实用户），但无法执行在线操作*/
	LoggedInLocalOnly,


	/** 状态或用户无效 */
	Invalid,
};

/** 判断用户可用的不同权限和功能 */
UENUM(BlueprintType)
enum class EModularUserPrivilege : uint8
{
	/** 用户是否可以在线或离线玩*/
	CanPlay,

	/** 用户是否可以在在线模式下玩 */
	CanPlayOnline,

	/** 用户是否可以使用文字聊天 */
	CanCommunicateViaTextOnline,

	/** 用户是否可以使用语音聊天 */
	CanCommunicateViaVoiceOnline,

	/**用户是否可以访问其他用户生成的内容 */
	CanUseUserGeneratedContent,

	/** 用户是否可以参与交叉游戏(多端游戏) */
	CanUseCrossPlay,

	/** 无效特权（以及有效特权的计数） */
	Invalid_Count UMETA(Hidden)
};

/** 枚举指定功能或特权的一般可用性，它结合了来自多个来源的信息 */
UENUM(BlueprintType)
enum class EModularUserAvailability : uint8
{
	/** 状态完全未知，需要查询*/
	Unknown,

	/** 此功能现在完全可用 */
	NowAvailable,

	/** 这可能在完成正常登录过程后可用 */
	PossiblyAvailable,

	/** 由于网络连接等原因，此功能现在不可用，但将来可能可用 */
	CurrentlyUnavailable,

	/** 由于硬帐户或平台限制，此功能在本次会话的剩余时间内永远不可用*/
	AlwaysUnavailable,

	/** 无效功能 */
	Invalid,
};

/** 枚举给出了用户可能使用或不使用特定权限的具体原因 */
UENUM(BlueprintType)
enum class EModularUserPrivilegeResult : uint8
{
	/** 状态未知，需要查询 */
	Unknown,

	/** 此特权完全可用 */
	Available,

	/** 用户尚未完全登录 */
	UserNotLoggedIn,

	/** 用户不拥有游戏或内容 */
	LicenseInvalid,

	/** 游戏需要更新或修补才能使用 */
	VersionOutdated,

	/** 没有网络连接，可以通过重新连接来解决 */
	NetworkConnectionUnavailable,

	/** 家长控制失败 */
	AgeRestricted,

	/** 帐户没有所需的订阅或帐户类型 */
	AccountTypeRestricted,

	/** 另一个帐户/用户限制，例如被服务禁止 */
	AccountUseRestricted,

	/** 其他平台特定故障 */
	PlatformFailure,
};

/** 用于跟踪联机时不同异步操作的进度 */
enum class EModularUserAsyncTaskState : uint8
{
	/** 任务尚未启动*/
	NotStarted,
	/** 任务当前正在处理中 */
	InProgress,
	/** 任务已成功完成 */
	Done,
	/** 任务未能完成 */
	Failed
};

/** 有关联机错误的详细信息。实际上是FOnlineError的包装器。 */
USTRUCT(BlueprintType)
struct FOnlineResultInformation
{
	GENERATED_BODY()

	/** 手术是否成功。如果成功，此结构的错误字段将不包含额外信息。 */
	UPROPERTY(BlueprintReadOnly)
	bool bWasSuccessful = true;

	/** 唯一的错误id可用于与特定处理的错误进行比较。 */
	UPROPERTY(BlueprintReadOnly)
	FString ErrorId;

	/** 要向用户显示的错误文本。 */
	UPROPERTY(BlueprintReadOnly)
	FText ErrorText;

	/**
	 *从FOnlineErrorType初始化此项
	 * @param InOnlineError 要从中初始化的联机错误
	 */
	void MODULARUSERMANAGEMENT_API FromOnlineError(const FOnlineErrorType& InOnlineError)
	{
#if MODULARUSER_OSSV1
		bWasSuccessful = InOnlineError.WasSuccessful();
		ErrorId = InOnlineError.GetErrorCode();
		ErrorText = InOnlineError.GetErrorMessage();
#endif
#if !MODULARUSER_OSSV1
		bWasSuccessful = InOnlineError != UE::Online::Errors::Success();
		ErrorId = InOnlineError.GetErrorId();
		ErrorText = InOnlineError.GetText();
#endif
	}
};
