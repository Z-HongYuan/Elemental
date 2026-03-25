// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "OnlineUserTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineUserSubsystem.generated.h"

#define UE_API ONLINEUSER_API

class FNativeGameplayTag;

/*构建子系统使用的GameplayTag*/
namespace OnlineUserTags
{
	// 常规严重程度级别和特定系统消息
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SystemMessage_Error);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SystemMessage_Warning)
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SystemMessage_Display)

	// 所有初始化玩家的尝试均失败，用户在重试前需要执行某些操作
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SystemMessage_Error_InitializeLocalPlayerFailed)


	// 平台特性标签，预期游戏实例或其他系统会为适当的平台调用 SetTraitTags 并传入这些标签

	/*此标签表示这是一个主机平台，直接将控制器 ID 映射到不同的系统用户。如果为 false，则同一用户可以拥有多个控制器*/
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Platform_Trait_RequiresStrictControllerMapping)

	/*此标签表示该平台只有一个在线用户，所有玩家都使用索引 0*/
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Platform_Trait_SingleOnlineUser)
}


/*单个用户的逻辑表示，每个已初始化的本地玩家都会存在一个这样的对象*/
UCLASS(MinimalAPI, BlueprintType)
class UOnlineUserInfo : public UObject
{
	GENERATED_BODY()

public:
	//该用户的主要控制器输入设备，他们可能还拥有额外的次要设备
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FInputDeviceId PrimaryInputDevice;

	//指定本地平台上的逻辑用户，客座用户将指向主用户
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FPlatformUserId PlatformUser;

	//如果此用户被分配了 LocalPlayer，一旦完全创建，这将匹配 GameInstance localplayers 数组中的索引
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	int32 LocalPlayerIndex = -1;

	//如果为 true，则允许此用户作为客座用户
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	bool bCanBeGuest = false;

	//如果为 true，则这是附加到主用户 0 的客座用户
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	bool bIsGuest = false;

	//用户初始化过程的整体状态
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	ECommonUserInitializationState InitializationState = ECommonUserInitializationState::Invalid;

	//如果此用户已成功登录，则返回 true
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API bool IsLoggedIn() const;

	//如果此用户正在登录过程中，则返回 true
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API bool IsDoingLogin() const;

	//返回特定权限的最新查询结果，如果从未查询过则返回 unknown
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API ECommonUserPrivilegeResult GetCachedPrivilegeResult(ECommonUserPrivilege Privilege, ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	//询问功能的总体可用性，这将结合缓存结果与当前状态
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API ECommonUserAvailability GetPrivilegeAvailability(ECommonUserPrivilege Privilege) const;

	//返回给定上下文的网络 ID
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FUniqueNetIdRepl GetNetId(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	//返回用户的可读昵称，这将返回在 UpdateCachedNetId 或 SetNickname 期间缓存的值
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FString GetNickname(ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game) const;

	//修改用户的可读昵称，这可用于设置多个客座用户，但对于真实用户会被平台昵称覆盖
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API void SetNickname(const FString& NewNickname, ECommonUserOnlineContext Context = ECommonUserOnlineContext::Game);

	//返回此玩家的内部调试字符串
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FString GetDebugString() const;

	//平台用户 ID 的访问器
	UE_API FPlatformUserId GetPlatformUserId() const;

	//获取平台用户索引，用于期望整数参数的旧函数
	UE_API int32 GetPlatformUserIndex() const;

	// 内部数据，仅旨在由在线子系统访问

	//每个在线系统的缓存数据
	struct FCachedData
	{
		//每个系统缓存的网络 ID
		FUniqueNetIdRepl CachedNetId;

		//缓存的昵称，每当网络 ID 可能更改时更新
		FString CachedNickname;

		//各种用户权限的缓存值
		TMap<ECommonUserPrivilege, ECommonUserPrivilegeResult> CachedPrivileges;
	};

	// 每个上下文的缓存，游戏上下文始终存在，但其他上下文可能不存在
	TMap<ECommonUserOnlineContext, FCachedData> CachedDataMap;

	//使用解析规则查找缓存数据
	UE_API FCachedData* GetCachedData(ECommonUserOnlineContext Context);
	UE_API const FCachedData* GetCachedData(ECommonUserOnlineContext Context) const;

	//更新缓存的权限结果，如果需要会传播到游戏上下文
	UE_API void UpdateCachedPrivilegeResult(ECommonUserPrivilege Privilege, ECommonUserPrivilegeResult Result, ECommonUserOnlineContext Context);

	//更新缓存的网络 ID，如果需要会传播到游戏上下文
	UE_API void UpdateCachedNetId(const FUniqueNetIdRepl& NewId, ECommonUserOnlineContext Context);

	//返回拥有此对象的子系统
	UE_API class UCommonUserSubsystem* GetSubsystem() const;
};


/**
 * 
 */
UCLASS()
class ONLINEUSER_API UOnlineUserSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
};

#undef UE_API
