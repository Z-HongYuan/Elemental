// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#define UE_API ONLINEUSER_API

/*这里是通过.Build文件中添加的定义宏*/
#if ONLINEUSER_OSSV1

// 在线子系统（OSS v1）包含并转发声明
#include "OnlineSubsystemTypes.h"
class IOnlineSubsystem;
struct FOnlineError;
using FOnlineErrorType = FOnlineError;
using ELoginStatusType = ELoginStatus::Type;

#else

// 在线服务（OSS v2）包括和转发声明
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

#endif


#include "OnlineUserTypes.generated.h"

/** 指定在线查询的地点和方式 */
UENUM(BlueprintType)
enum class ECommonUserOnlineContext : uint8
{
	/** 它从游戏代码调用，使用默认系统，但带有特殊处理，可以合并多个上下文的结果 */
	Game,

	/** 默认的引擎在线系统，它将永远存在，且与服务或平台相同 */
	Default,

	/** 明确要求外部服务，虽然可能不存在 */
	Service,

	/** 先查找外部服务，然后恢复默认 */
	ServiceOrDefault,

	/** 明确要求平台系统，虽然平台系统可能不存在 */
	Platform,

	/** 先找平台系统，然后恢复默认 */
	PlatformOrDefault,

	/** 无效系统 */
	Invalid
};

/** 描述特定用户初始化状态的枚举 */
UENUM(BlueprintType)
enum class ECommonUserInitializationState : uint8
{
	/** 用户尚未开始登录流程 */
	Unknown,

	/** 玩家正在获取本地登录的用户id */
	DoingInitialLogin,

	/** 玩家正在进行网络登录，他们已经在本地登录 */
	DoingNetworkLogin,

	/** 玩家根本无法登录 */
	FailedtoLogin,


	/** 玩家已登录并可以访问在线功能 */
	LoggedInOnline,

	/** 玩家已在本地登录（无论是访客还是真实用户），但无法执行在线操作 */
	LoggedInLocalOnly,


	/** 状态或用户无效 */
	Invalid,
};

/** 枚举指定用户可用的不同权限和功能 */
UENUM(BlueprintType)
enum class ECommonUserPrivilege : uint8
{
	/** 用户是否可以在线或离线玩 */
	CanPlay,

	/** 用户是否可以在在线模式下玩 */
	CanPlayOnline,

	/** 用户是否可以使用文字聊天 */
	CanCommunicateViaTextOnline,

	/** 用户是否可以使用语音聊天 */
	CanCommunicateViaVoiceOnline,

	/** 用户是否可以访问其他用户生成的内容 */
	CanUseUserGeneratedContent,

	/** 用户是否可以参与交叉游戏 */
	CanUseCrossPlay,

	/** 无效特权（以及有效特权的计数） */
	Invalid_Count UMETA(Hidden)
};

/** 枚举指定功能或特权的一般可用性，它结合了来自多个来源的信息 */
UENUM(BlueprintType)
enum class ECommonUserAvailability : uint8
{
	/** 状态完全未知，需要查询 */
	Unknown,

	/** 此功能现在完全可用 */
	NowAvailable,

	/** 这可能在完成正常登录过程后可用 */
	PossiblyAvailable,

	/** 由于网络连接等原因，此功能现在不可用，但将来可能可用 */
	CurrentlyUnavailable,

	/** 由于硬账户或平台限制，此功能在本次会话的剩余时间内永远不可用 */
	AlwaysUnavailable,

	/** 无效功能 */
	Invalid,
};

/** 枚举给出了用户可能使用或不使用特定权限的具体原因 */
UENUM(BlueprintType)
enum class ECommonUserPrivilegeResult : uint8
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

	/** 账户没有所需的订阅或账户类型 */
	AccountTypeRestricted,

	/** 另一个账户/用户限制，例如被服务禁止 */
	AccountUseRestricted,

	/** 其他平台特定故障 */
	PlatformFailure,
};

/** 用于跟踪不同异步操作的进度 */
enum class ECommonUserAsyncTaskState : uint8
{
	/** 任务尚未启动 */
	NotStarted,
	/** 任务当前正在处理中 */
	InProgress,
	/** 任务已成功完成 */
	Done,
	/** 任务未能完成 */
	Failed
};

/** 有关联机错误的详细信息。有效地包装FOnlineError. */
USTRUCT(BlueprintType)
struct FOnlineResultInformation
{
	GENERATED_BODY()

	/** 手术是否成功。如果成功，此结构的错误字段将不包含额外信息。 */
	UPROPERTY(BlueprintReadOnly)
	bool bWasSuccessful = true;

	/** 唯一的错误id。可用于与特定处理的错误进行比较. */
	UPROPERTY(BlueprintReadOnly)
	FString ErrorId;

	/** 要向用户显示的错误文本。 */
	UPROPERTY(BlueprintReadOnly)
	FText ErrorText;

	/**
	 * 从FOnlineErrorType初始化此项
	 * 转换/初始化函数，用于将引擎的底层在线错误对象（FOnlineErrorType）转换为项目自定义的错误信息结构（FOnlineResultInformation）
	 * 通过中间层来处理，因此项目可以轻松地替换底层引擎对象,轻松应对底层API改变
	 * @param InOnlineError 初始化时发生的在线错误
	 */
	void UE_API FromOnlineError(const FOnlineErrorType& InOnlineError);
};

#undef UE_API
