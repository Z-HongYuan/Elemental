// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "Engine/GameInstance.h"
#include "ModularGameInstance.generated.h"

#define UE_API MODULARGAMEHUB_API

/**
 * 
 */
UCLASS(MinimalAPI, Abstract, Config = Game)
class UModularGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// /** 处理来自 ModularUserManagement 的错误/警告，可以按游戏覆盖 */
	// UFUNCTION()
	// UE_API virtual void HandleSystemMessage(FGameplayTag MessageType, FText Title, FText Message);
	//
	// UFUNCTION()
	// UE_API virtual void HandlePrivilegeChanged(const UModularUserInfo* UserInfo, ECommonUserPrivilege Privilege, ECommonUserAvailability OldAvailability, ECommonUserAvailability NewAvailability);
	//
	// UFUNCTION()
	// UE_API virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);
	//
	// /** 重置用户和会话状态，通常是因为玩家已断开连接 */
	// UE_API virtual void ResetUserAndSessionState();
	//
	// /**
	//  *请求的会话流
	//  *某物请求用户加入特定会话（例如，通过OnUserRequestedSession的平台覆盖）。
	//  *此请求在SetRequestedSession中处理。
	//  *检查我们是否可以立即加入请求的会话（CanJoinRequestedSession）。如果可以，加入请求的会话（JoinRequestedSession）
	//  *如果没有，缓存请求的会话并指示游戏进入可以加入会话的状态（ResetGameAndJoinRequestedSession）
	//  */
	// /** 处理用户接受来自外部源（例如，平台覆盖）的会话邀请。每个游戏都会被覆盖。 */
	// UE_API virtual void OnUserRequestedSession(const FPlatformUserId& PlatformUserId, UCommonSession_SearchResult* InRequestedSession, const FOnlineResultInformation& RequestedSessionResult);
	//
	// /** 处理OSS请求销毁会话 */
	// UE_API virtual void OnDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName);
	//
	// /** 获取请求的会话 */
	// UCommonSession_SearchResult* GetRequestedSession() const { return RequestedSession; }
	// /** 设置（或清除）请求的会话。设置此选项后，请求的会话流开始。 */
	// UE_API virtual void SetRequestedSession(UCommonSession_SearchResult* InRequestedSession);
	// /** 检查请求的会话是否可以加入。每个游戏都可以被覆盖。 */
	// UE_API virtual bool CanJoinRequestedSession() const;
	// /**加入请求的会话 */
	// UE_API virtual void JoinRequestedSession();
	// /** 使游戏进入加入所请求会话的状态 */
	// UE_API virtual void ResetGameAndJoinRequestedSession();

	
	UE_API virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId) override;
	UE_API virtual bool RemoveLocalPlayer(ULocalPlayer* ExistingPlayer) override;
	UE_API virtual void Init() override;
	UE_API virtual void ReturnToMainMenu() override;

private:
	/** 这是主要玩家 */
	TWeakObjectPtr<ULocalPlayer> PrimaryPlayer;
	
	// /** 玩家请求加入的会话 */
	// UPROPERTY()
	// TObjectPtr<UCommonSession_SearchResult> RequestedSession;
};

#undef UE_API
