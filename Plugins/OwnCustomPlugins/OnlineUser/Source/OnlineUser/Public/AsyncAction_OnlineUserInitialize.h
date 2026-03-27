// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineUserSubsystem.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_OnlineUserInitialize.generated.h"

#define UE_API ONLINEUSER_API

class UOnlineUserSubsystem;
/**
 * 异步操作，用于处理初始化用户的不同功能
 */
UCLASS()
class UAsyncAction_OnlineUserInitialize : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * 使用通用用户系统初始化本地播放器，其中包括执行特定于平台的登录和权限检查。
     * 当进程成功或失败时，它将广播OnInitialization Complete委托。
	 *
	 * @param LocalPlayerIndex	游戏实例中ULocalPlayer的期望索引，0将是主要玩家，1+用于本地多人游戏
	 * @param PrimaryInputDevice 用户的主要输入设备，如果无效，将使用系统默认值
	 * @param bCanUseGuestLogin	如果为真，则该玩家可以是没有真实系统网络id的访客
	 */
	UFUNCTION(BlueprintCallable, Category = OnlineUser, meta = (BlueprintInternalUseOnly = "true"))
	static UE_API UAsyncAction_OnlineUserInitialize* InitializeForLocalPlay(UOnlineUserSubsystem* Target, int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin);

	/**
	 * 尝试将现有用户登录到特定于平台的在线后端，以启用完整的在线游戏
	 * 当进程成功或失败时，它将广播OnInitialization Complete委托。
	 *
	 * @param LocalPlayerIndex	游戏实例中现有LocalPlayer的索引
	 */
	UFUNCTION(BlueprintCallable, Category = OnlineUser, meta = (BlueprintInternalUseOnly = "true"))
	static UE_API UAsyncAction_OnlineUserInitialize* LoginForOnlinePlay(UOnlineUserSubsystem* Target, int32 LocalPlayerIndex);

	/** 初始化成功或失败时调用 */
	UPROPERTY(BlueprintAssignable)
	FOnlineUserOnInitializeCompleteMulticast OnInitializationComplete;

	/**失败并在需要时发送回调 */
	UE_API void HandleFailure();

	/**包装器委托，如果合适，将传递给OnInitialization Complete */
	UFUNCTION()
	UE_API virtual void HandleInitializationComplete(const UOnlineUserInfo* UserInfo, bool bSuccess, FText Error, EOnlineUserPrivilege RequestedPrivilege, EOnlineUserOnlineContext OnlineContext);

protected:
	/** 实际开始初始化 */
	UE_API virtual void Activate() override;

	TWeakObjectPtr<UOnlineUserSubsystem> Subsystem;
	FOnlineUserInitializeParams Params;
};

#undef UE_API
