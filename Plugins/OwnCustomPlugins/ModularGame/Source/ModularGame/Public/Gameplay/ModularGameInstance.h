// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ModularGameInstance.generated.h"

#define UE_API MODULARGAME_API

struct FGameplayTag;
enum class EOnlineUserAvailability : uint8;
enum class EOnlineUserOnlineContext : uint8;
enum class EOnlineUserPrivilege : uint8;
class UOnlineUserInfo;
struct FOnlineResultInformation;
class UOnlineSession_SearchResult;

/**
 * 
 */
UCLASS(MinimalAPI, Abstract, Config = Game)
class UModularGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UE_API UModularGameInstance(const FObjectInitializer& ObjectInitializer);

	/** 处理来自 CommonUser 的错误/警告，可针对每个游戏进行重写。 */
	UFUNCTION()
	UE_API virtual void HandleSystemMessage(FGameplayTag MessageType, FText Title, FText Message);

	UFUNCTION()
	UE_API virtual void HandlePrivilegeChanged(const UOnlineUserInfo* UserInfo, EOnlineUserPrivilege Privilege, EOnlineUserAvailability OldAvailability, EOnlineUserAvailability NewAvailability);

	UFUNCTION()
	UE_API virtual void HandlerUserInitialized(const UOnlineUserInfo* UserInfo, bool bSuccess, FText Error, EOnlineUserPrivilege RequestedPrivilege, EOnlineUserOnlineContext OnlineContext);

	/** 调用此命令重置用户和会话状态，通常是因为玩家已断开连接。*/
	UE_API virtual void ResetUserAndSessionState();

	/**
	* 请求会话流程
	* 某个程序请求用户加入特定会话（例如，平台覆盖层通过 OnUserRequestedSession 函数发出请求）。
	* 此请求在 SetRequestedSession 函数中处理。
	* 检查是否可以立即加入请求的会话 (CanJoinRequestedSession)。如果可以，则加入请求的会话 (JoinRequestedSession)。
	* 如果不能，则缓存请求的会话，并指示游戏进入可以加入该会话的状态 (ResetGameAndJoinRequestedSession)。
	*/
	/** 处理用户接受来自外部来源（例如平台叠加层）的会话邀请。此设置可针对每个游戏进行单独重写。*/
	UE_API virtual void OnUserRequestedSession(const FPlatformUserId& PlatformUserId, UOnlineSession_SearchResult* InRequestedSession, const FOnlineResultInformation& RequestedSessionResult);

	/**处理 OSS 请求，即销毁会话。*/
	UE_API virtual void OnDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName);

	/** 获取请求的会话*/
	UOnlineSession_SearchResult* GetRequestedSession() const { return RequestedSession; }
	/** 设置（或清除）请求的会话。设置完成后，请求的会话流程即会启动。 */
	UE_API virtual void SetRequestedSession(UOnlineSession_SearchResult* InRequestedSession);
	/** 检查请求的会话是否可以加入。可以针对每个游戏进行单独设置。 */
	UE_API virtual bool CanJoinRequestedSession() const;
	/** 加入请求的会话*/
	UE_API virtual void JoinRequestedSession();
	/** 使游戏处于可以加入所请求会话的状态。 */
	UE_API virtual void ResetGameAndJoinRequestedSession();

	UE_API virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId) override;
	UE_API virtual bool RemoveLocalPlayer(ULocalPlayer* ExistingPlayer) override;
	UE_API virtual void Init() override;
	UE_API virtual void ReturnToMainMenu() override;

private:
	/**这是主要玩家*/
	TWeakObjectPtr<ULocalPlayer> PrimaryPlayer;
	/** 玩家请求加入的会话 */
	UPROPERTY()
	TObjectPtr<UOnlineSession_SearchResult> RequestedSession;
};
#undef UE_API
