// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "ModularSession_HostSessionRequest.h"
#include "ModularSession_SearchSessionRequest.h"
#include "OnlineBeaconHost.h"
#include "PartyBeaconClient.h"
#include "PartyBeaconState.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UserManagement/ModularUserTypes.h"

#if MODULARUSER_OSSV1
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#endif

#if !MODULARUSER_OSSV1
#include "Online/Sessions.h"
#include "Online/Lobbies.h"
#include "Online/OnlineAsyncOpHandle.h"
#endif

#include "ModularUserSessionManager.generated.h"

#if MODULARUSER_OSSV1
class FModularOnlineSearchSettingsOSSv1;
using FModularOnlineSearchSettings = FModularOnlineSearchSettingsOSSv1;
#endif

#if !MODULARUSER_OSSV1
class FModularOnlineSearchSettingsOSSv2;
using FModularOnlineSearchSettings = FModularOnlineSearchSettingsOSSv2;
#endif


#define UE_API MODULARUSERMANAGEMENT_API

/**
*当本地用户请求从外部源（例如从平台覆盖）加入会话时触发的事件。
*一般来说，游戏应该让玩家进入会话。
*@param LocalPlatformUserId 接受邀请的本地用户id。这是一个平台用户id，因为该用户可能尚未登录。
*@param RequestedSession 请求的会话。如果处理请求时出错，则可以为null。
*@param RequestedSession 请求会话处理的结果
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FModularSessionOnUserRequestedSession, const FPlatformUserId& /*LocalPlatformUserId*/, UModularSession_SearchResult* /*RequestedSession*/, const FOnlineResultInformation& /*RequestedSessionResult*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FModularSessionOnUserRequestedSession_Dynamic, const FPlatformUserId&, LocalPlatformUserId, UModularSession_SearchResult*, RequestedSession, const FOnlineResultInformation&,
                                               RequestedSessionResult);

/**
*当会话加入完成时触发的事件，在加入基础会话之后，如果成功则在前往服务器之前触发。
*事件参数指示此操作是否成功，或者是否存在将阻止其运行的错误。
*@param 会话加入的结果
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FModularSessionOnJoinSessionComplete, const FOnlineResultInformation& /*Result*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FModularSessionOnJoinSessionComplete_Dynamic, const FOnlineResultInformation&, Result);

/**
*当托管会话创建完成时触发的事件，就在它到达地图之前。
*事件参数指示此操作是否成功，或者是否存在将阻止其运行的错误。
*@param 会话加入的结果
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FModularSessionOnCreateSessionComplete, const FOnlineResultInformation& /*Result*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FModularSessionOnCreateSessionComplete_Dynamic, const FOnlineResultInformation&, Result);

/**
*当本地用户请求从外部源（例如平台覆盖）销毁会话时触发的事件。
*游戏应该让玩家退出会话。
*@param LocalPlatformUserId发出销毁请求的本地用户id。这是一个平台用户id，因为该用户可能尚未登录。
*@param SessionName会话的名称标识符。
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FModularSessionOnDestroySessionRequested, const FPlatformUserId& /*LocalPlatformUserId*/, const FName& /*SessionName*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FModularSessionOnDestroySessionRequested_Dynamic, const FPlatformUserId&, LocalPlatformUserId, const FName&, SessionName);

/**
*在解析连接字符串之后和客户端移动之前，会话加入完成时触发的事件。
*@param URL解析了会话的连接字符串以及任何其他参数
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FModularSessionOnPreClientTravel, FString& /*URL*/);

/**
*在会话生态系统中的不同点触发的事件，表示会话的用户可呈现状态。
*这不应用于在线功能（对于这些功能，请使用OnCreateSessionComplete或OnJoinSessionComplete），而是用于丰富的在线状态等功能
 */
UENUM(BlueprintType)
enum class EModularSessionInformationState : uint8
{
	OutOfGame,
	Matchmaking,
	InGame
};

DECLARE_MULTICAST_DELEGATE_ThreeParams(FModularSessionOnSessionInformationChanged, EModularSessionInformationState /*SessionStatus*/, const FString& /*GameMode*/, const FString& /*MapName*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FModularSessionOnSessionInformationChanged_Dynamic, EModularSessionInformationState, SessionStatus, const FString&, GameMode, const FString&, MapName);


/**
* 处理托管和加入在线游戏请求的游戏子系统。
*为每个游戏实例创建一个子系统，可以从蓝图或C++代码访问。
*如果存在特定于游戏的子类，则不会创建此基本子系统。
 */
UCLASS(MinimalAPI, BlueprintType, Config=Engine)
class UModularUserSessionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/** 使用在线游戏的默认选项创建主机会话请求,此请求可以在创建后修改 */
	UFUNCTION(BlueprintCallable, Category = Session)
	UE_API virtual UModularSession_HostSessionRequest* CreateOnlineHostSessionRequest();

	/** 创建一个具有默认选项的会话搜索对象以查找默认在线游戏,此对象可以在创建后修改 */
	UFUNCTION(BlueprintCallable, Category = Session)
	UE_API virtual UModularSession_SearchSessionRequest* CreateOnlineSearchSessionRequest();

	/** 使用会话请求信息创建新的在线游戏,如果成功将开始硬地图转移 */
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API virtual void HostSession(APlayerController* HostingPlayer, UModularSession_HostSessionRequest* Request);

	/** 启动一个流程来查找现有会话,如果未找到可行的会话则创建新会话 */
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API virtual void QuickPlaySession(APlayerController* JoiningOrHostingPlayer, UModularSession_HostSessionRequest* Request);

	/** 启动加入现有会话的流程,如果成功将连接到指定的服务器 */
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API virtual void JoinSession(APlayerController* JoiningPlayer, UModularSession_SearchResult* Request);

	/** 向在线系统查询与搜索请求匹配的可加入会话列表 */
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API virtual void FindSessions(APlayerController* SearchingPlayer, UModularSession_SearchSessionRequest* Request);

	/** 清理任何活动会话,从返回主菜单等情况下调用 */
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API virtual void CleanUpSessions();

	//////////////////////////////////////////////////////////////////////
	// 事件

	/** 当本地用户接受邀请时的原生委托 */
	FModularSessionOnUserRequestedSession OnUserRequestedSessionEvent;
	/** 当本地用户接受邀请时广播的事件 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On User Requested Session"))
	FModularSessionOnUserRequestedSession_Dynamic K2_OnUserRequestedSessionEvent;

	/** 当JoinSession调用完成时的原生委托 */
	FModularSessionOnJoinSessionComplete OnJoinSessionCompleteEvent;
	/** 当JoinSession调用完成时广播的事件 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Join Session Complete"))
	FModularSessionOnJoinSessionComplete_Dynamic K2_OnJoinSessionCompleteEvent;

	/** 当CreateSession调用完成时的原生委托 */
	FModularSessionOnCreateSessionComplete OnCreateSessionCompleteEvent;
	/** 当CreateSession调用完成时广播的事件 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Create Session Complete"))
	FModularSessionOnCreateSessionComplete_Dynamic K2_OnCreateSessionCompleteEvent;

	/** 当可展示的会话信息发生变化时的原生委托 */
	FModularSessionOnSessionInformationChanged OnSessionInformationChangedEvent;
	/** 当可展示的会话信息发生变化时广播的事件 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Session Information Changed"))
	FModularSessionOnSessionInformationChanged_Dynamic K2_OnSessionInformationChangedEvent;

	/** 当请求销毁平台会话时的原生委托 */
	FModularSessionOnDestroySessionRequested OnDestroySessionRequestedEvent;
	/** 当请求销毁平台会话时广播的事件 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Leave Session Requested"))
	FModularSessionOnDestroySessionRequested_Dynamic K2_OnDestroySessionRequestedEvent;

	/** 在客户端移动之前修改连接URL的原生委托 */
	FModularSessionOnPreClientTravel OnPreClientTravelEvent;

	// 配置设置,这些可以在子类或配置文件中被覆盖

	/** 设置会话搜索和主机请求中bUseLobbies的默认值 */
	UPROPERTY(Config)
	bool bUseLobbiesDefault = true;

	/** 设置会话主机请求中bUseLobbiesVoiceChat的默认值 */
	UPROPERTY(Config)
	bool bUseLobbiesVoiceChatDefault = false;

	/** 在创建或加入游戏会话时,在服务器转移之前启用预留信标流程 */
	UPROPERTY(Config)
	bool bUseBeacons = true;

protected:
	// 在创建或加入会话过程中调用的函数,这些可以被覆盖以实现特定于游戏的行为

	/** 调用以从快速播放主机设置填充会话请求,可以覆盖以实现特定于游戏的行为 */
	UE_API virtual TSharedRef<FModularOnlineSearchSettings> CreateQuickPlaySearchSettings(UModularSession_HostSessionRequest* Request, UModularSession_SearchSessionRequest* QuickPlayRequest);

	/** 当快速播放搜索完成时调用,可以覆盖以实现特定于游戏的行为 */
	UE_API virtual void HandleQuickPlaySearchFinished(bool bSucceeded, const FText& ErrorMessage, TWeakObjectPtr<APlayerController> JoiningOrHostingPlayer, TStrongObjectPtr<UModularSession_HostSessionRequest> HostRequest);

	/** 当转移到会话失败时调用 */
	UE_API virtual void TravelLocalSessionFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ReasonString);

	/** 当新会话创建成功或失败时调用 */
	UE_API virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/** 调用以完成会话创建 */
	UE_API virtual void FinishSessionCreation(bool bWasSuccessful);

	/** 在转移到新的托管会话地图后调用 */
	UE_API virtual void HandlePostLoadMap(UWorld* World);

protected:
	// 用于初始化和处理在线系统结果的内部函数

	UE_API void BindOnlineDelegates();
	UE_API void CreateOnlineSessionInternal(ULocalPlayer* LocalPlayer, UModularSession_HostSessionRequest* Request);
	UE_API void FindSessionsInternal(APlayerController* SearchingPlayer, const TSharedRef<FModularOnlineSearchSettings>& InSearchSettings);
	UE_API void JoinSessionInternal(ULocalPlayer* LocalPlayer, UModularSession_SearchResult* Request);
	UE_API void InternalTravelToSession(const FName SessionName);
	UE_API void NotifyUserRequestedSession(const FPlatformUserId& PlatformUserId, UModularSession_SearchResult* RequestedSession, const FOnlineResultInformation& RequestedSessionResult);
	UE_API void NotifyJoinSessionComplete(const FOnlineResultInformation& Result);
	UE_API void NotifyCreateSessionComplete(const FOnlineResultInformation& Result);
	UE_API void NotifySessionInformationUpdated(EModularSessionInformationState SessionStatusStr, const FString& GameMode = FString(), const FString& MapName = FString());
	UE_API void NotifyDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName);
	UE_API void SetCreateSessionError(const FText& ErrorText);

#if MODULARUSER_OSSV1
	UE_API void BindOnlineDelegatesOSSv1();
	UE_API void CreateOnlineSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UModularSession_HostSessionRequest* Request);
	UE_API void FindSessionsInternalOSSv1(ULocalPlayer* LocalPlayer);
	UE_API void JoinSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UModularSession_SearchResult* Request);
	UE_API TSharedRef<FModularOnlineSearchSettings> CreateQuickPlaySearchSettingsOSSv1(UModularSession_HostSessionRequest* Request, UModularSession_SearchSessionRequest* QuickPlayRequest);
	UE_API void CleanUpSessionsOSSv1();

	UE_API void HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType);
	UE_API void HandleSessionUserInviteAccepted(const bool bWasSuccessful, const int32 LocalUserIndex, FUniqueNetIdPtr AcceptingUserId, const FOnlineSessionSearchResult& SearchResult);
	UE_API void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	UE_API void OnRegisterLocalPlayerComplete_CreateSession(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);
	UE_API void OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful);
	UE_API void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);
	UE_API void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	UE_API void OnDestroySessionRequested(int32 LocalUserNum, FName SessionName);
	UE_API void OnFindSessionsComplete(bool bWasSuccessful);
	UE_API void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	UE_API void OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);
	UE_API void FinishJoinSession(EOnJoinSessionCompleteResult::Type Result);

#endif

#if !MODULARUSER_OSSV1
	UE_API void BindOnlineDelegatesOSSv2();
	UE_API void CreateOnlineSessionInternalOSSv2(ULocalPlayer* LocalPlayer, UModularSession_HostSessionRequest* Request);
	UE_API void FindSessionsInternalOSSv2(ULocalPlayer* LocalPlayer);
	UE_API void JoinSessionInternalOSSv2(ULocalPlayer* LocalPlayer, UModularSession_SearchResult* Request);
	UE_API TSharedRef<FModularOnlineSearchSettings> CreateQuickPlaySearchSettingsOSSv2(UModularSession_HostSessionRequest* HostRequest, UModularSession_SearchSessionRequest* SearchRequest);
	UE_API void CleanUpSessionsOSSv2();

	/** 处理源自在线服务的加入请求 */
	UE_API void OnLobbyJoinRequested(const UE::Online::FUILobbyJoinRequested& EventParams);

	/** 处理源自在线服务的SESSION加入请求 */
	UE_API void OnSessionJoinRequested(const UE::Online::FUISessionJoinRequested& EventParams);

	/** 获取给定控制器的本地用户ID */
	UE_API UE::Online::FAccountId GetAccountId(APlayerController* PlayerController) const;
	/** 获取给定会话名称的大厅ID */
	UE_API UE::Online::FLobbyId GetLobbyId(const FName SessionName) const;
	/** UI大厅加入请求的事件句柄 */
	UE::Online::FOnlineEventDelegateHandle LobbyJoinRequestedHandle;

	/** UI大厅会话请求的事件句柄 */
	UE::Online::FOnlineEventDelegateHandle SessionJoinRequestedHandle;

#endif

	UE_API void CreateHostReservationBeacon();
	UE_API void ConnectToHostReservationBeacon();
	UE_API void DestroyHostReservationBeacon();

protected:
	/** 会话操作完成后将使用的转移URL */
	FString PendingTravelURL;

	/** 会话创建尝试的最新结果信息,存储在此处以便稍后存储错误代码 */
	FOnlineResultInformation CreateSessionResult;

	/** 如果我们想在会话创建后取消它,则为true */
	bool bWantToDestroyPendingSession = false;

	/** 如果这是专用服务器(不需要LocalPlayer来创建会话),则为true */
	bool bIsDedicatedServer = false;

	/** 当前搜索的设置 */
	TSharedPtr<FModularOnlineSearchSettings> SearchSettings;

	/** 用于注册信标的通用信标监听器 */
	UPROPERTY(Transient)
	TWeakObjectPtr<AOnlineBeaconHost> BeaconHostListener;
	/** 信标主机的状态 */
	UPROPERTY(Transient)
	TObjectPtr<UPartyBeaconState> ReservationBeaconHostState;
	/** 控制对此游戏访问的信标 */
	UPROPERTY(Transient)
	TWeakObjectPtr<APartyBeaconHost> ReservationBeaconHost;
	/** 用于信标通信的模块化类对象 */
	UPROPERTY(Transient)
	TWeakObjectPtr<APartyBeaconClient> ReservationBeaconClient;

	/** 信标预留的团队数量 */
	UPROPERTY(Config)
	int32 BeaconTeamCount = 2;
	/** 信标预留的团队大小 */
	UPROPERTY(Config)
	int32 BeaconTeamSize = 8;
	/** 最大信标预留数量 */
	UPROPERTY(Config)
	int32 BeaconMaxReservations = 16;
};

#undef UE_API
