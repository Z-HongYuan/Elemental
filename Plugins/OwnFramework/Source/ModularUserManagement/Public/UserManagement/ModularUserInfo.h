// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "ModularUserTypes.h"
#include "UObject/Object.h"
#include "ModularUserInfo.generated.h"

#define UE_API MODULARUSERMANAGEMENT_API

/**
 * 用户信息对象
 * 对于所有初始化的用户中,都会创建一个对象保存用户信息
 */
UCLASS(MinimalAPI, BlueprintType)
class UModularUserInfo : public UObject
{
	GENERATED_BODY()

public:
	/** 此用户的主控制器输入设备，也可以有其他辅助设备 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FInputDeviceId PrimaryInputDevice;

	/** 指定本地平台上的逻辑用户，游客用户将指向主用户*/
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FPlatformUserId PlatformUser;

	/** 如果为该用户分配了LocalPlayer，则一旦完全创建，它将与GameInstance localplayers数组中的索引匹配 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	int32 LocalPlayerIndex = -1;

	/**如果为真，则允许此用户作为游客 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	bool bCanBeGuest = false;

	/** 如果为真，则这是一个附加到主用户0的游客用户*/
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	bool bIsGuest = false;

	/** 用户初始化过程的总体状态 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	EModularUserInitializationState InitializationState = EModularUserInitializationState::Invalid;

	/** 如果此用户已成功登录，则返回true */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API bool IsLoggedIn() const;

	/** 如果此用户正在登录，则返回true */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API bool IsDoingLogin() const;

	/** 返回特定权限的最近查询结果，如果从未查询过，将返回未知 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API EModularUserPrivilegeResult GetCachedPrivilegeResult(EModularUserPrivilege Privilege, EModularUserOnlineContext Context = EModularUserOnlineContext::Game) const;

	/** 询问功能的一般可用性，这将缓存结果与状态相结合 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API EModularUserAvailability GetPrivilegeAvailability(EModularUserPrivilege Privilege) const;

	/**返回给定上下文的网络id */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FUniqueNetIdRepl GetNetId(EModularUserOnlineContext Context = EModularUserOnlineContext::Game) const;

	/** 返回用户可读的昵称，这将返回在UpdateCachedNetId或SetNickname期间缓存的值 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FString GetNickname(EModularUserOnlineContext Context = EModularUserOnlineContext::Game) const;

	/**修改用户可读的昵称，这可以在设置多个访客时使用，但会被真实用户的平台昵称覆盖 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API void SetNickname(const FString& NewNickname, EModularUserOnlineContext Context = EModularUserOnlineContext::Game);

	/** 返回此播放器的内部调试字符串 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FString GetDebugString() const;

	/** 平台用户id的访问器 */
	UE_API FPlatformUserId GetPlatformUserId() const;

	/** 获取需要整数的旧函数的平台用户索引*/
	UE_API int32 GetPlatformUserIndex() const;

	// 内部数据，仅供在线子系统访问

	/** 每个在线系统的缓存数据 */
	struct FCachedData
	{
		/** 每个系统的缓存网络id */
		FUniqueNetIdRepl CachedNetId;

		/** 缓存的nickanem，在网络ID可能更改时更新 */
		FString CachedNickname;

		/** 各种用户权限的缓存值 */
		TMap<EModularUserPrivilege, EModularUserPrivilegeResult> CachedPrivileges;
	};

	/** 根据上下文缓存，游戏将始终存在，但其他游戏可能不存在 */
	TMap<EModularUserOnlineContext, FCachedData> CachedDataMap;

	/** 使用解析规则查找缓存数据 */
	UE_API FCachedData* GetCachedData(EModularUserOnlineContext Context);
	UE_API const FCachedData* GetCachedData(EModularUserOnlineContext Context) const;

	/** 更新缓存的权限结果，如果需要，将传播到游戏 */
	UE_API void UpdateCachedPrivilegeResult(EModularUserPrivilege Privilege, EModularUserPrivilegeResult Result, EModularUserOnlineContext Context);

	/** 更新缓存的权限结果，如果需要，将传播到游戏 */
	UE_API void UpdateCachedNetId(const FUniqueNetIdRepl& NewId, EModularUserOnlineContext Context);

	/** 返回其所属的子系统 */
	UE_API class UModularUserManager* GetSubsystem() const;
};

/** 初始化过程成功或失败时的代理 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FModularUserOnInitializeCompleteMulticast,
                                              const UModularUserInfo*, UserInfo,
                                              bool, bSuccess,
                                              FText, Error,
                                              EModularUserPrivilege, RequestedPrivilege,
                                              EModularUserOnlineContext, OnlineContext);

DECLARE_DYNAMIC_DELEGATE_FiveParams(FModularUserOnInitializeComplete,
                                    const UModularUserInfo*, UserInfo,
                                    bool, bSuccess,
                                    FText, Error,
                                    EModularUserPrivilege, RequestedPrivilege,
                                    EModularUserOnlineContext, OnlineContext);

/** 当发送系统错误消息时，游戏可以选择使用类型标签将其显示给用户 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FModularUserHandleSystemMessageDelegate,
                                               FGameplayTag, MessageType,
                                               FText, TitleText,
                                               FText, BodyText);

/** 当特权发生变化时，可以委托他人查看在线状态等在游戏过程中是否发生变化*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FModularUserAvailabilityChangedDelegate,
                                              const UModularUserInfo*, UserInfo,
                                              EModularUserPrivilege, Privilege,
                                              EModularUserAvailability, OldAvailability,
                                              EModularUserAvailability, NewAvailability);


/** 参数结构用于初始化函数，这通常由异步节点等包装函数填充 */
USTRUCT(BlueprintType)
struct FModularUserInitializeParams
{
	GENERATED_BODY()

	/** 要使用的本地玩家索引，可以指定一个高于当前启用的创建玩家索引*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	int32 LocalPlayerIndex = 0;

	/** 已弃用的选择平台用户和输入设备的方法 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	int32 ControllerId = -1;

	/**此用户的主控制器输入设备，也可以有其他辅助设备*/
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FInputDeviceId PrimaryInputDevice;

	/** 指定本地平台上的逻辑用户 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FPlatformUserId PlatformUser;

	/** 通常，CanPlay或CanPlayOnline会指定所需的权限级别 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	EModularUserPrivilege RequestedPrivilege = EModularUserPrivilege::CanPlay;

	/** 登录到哪个特定的在线环境，游戏意味着登录到所有相关的环境 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	EModularUserOnlineContext OnlineContext = EModularUserOnlineContext::Game;

	/** 如果允许创建新的本地玩家进行初始登录，则为True */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	bool bCanCreateNewLocalPlayer = false;

	/** 如果此玩家可以是没有实际在线状态的访客用户，则为True */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	bool bCanUseGuestLogin = false;

	/** 如果我们不显示登录错误，游戏将负责显示这些错误 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	bool bSuppressLoginErrors = false;

	/**如果绑定，请在登录完成时调用此动态委托 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	FModularUserOnInitializeComplete OnUserInitializeComplete;
};

#undef UE_API
