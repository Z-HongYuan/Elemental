// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineBeaconHost.h"
#include "OnlineSessionSettings.h"
#include "OnlineUserTypes.h"
#include "PartyBeaconHost.h"
#include "Online/OnlineSessionNames.h"
#include "Subsystems/GameInstanceSubsystem.h"


//根据不同的版本,使用不同的头文件和别名
#if ONLINEUSER_OSSV1
inline FName SETTING_ONLINESUBSYSTEM_VERSION(TEXT("OSSv1"));
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
class FOnlineOnlineSearchSettingsOSSv1;
using FOnlineOnlineSearchSettings = FOnlineOnlineSearchSettingsOSSv1;
#endif

#if !ONLINEUSER_OSSV1
inline FName SETTING_ONLINESUBSYSTEM_VERSION(TEXT("OSSv2"));
#include "Online/Lobbies.h"
#include "Online/OnlineAsyncOpHandle.h"
#include "Online/Sessions.h"
class FOnlineOnlineSearchSettingsOSSv2;
using FOnlineOnlineSearchSettings = FOnlineOnlineSearchSettingsOSSv2;
#endif

#include "OnlineSessionSubsystem.generated.h"

#define UE_API ONLINEUSER_API


/** 规定游戏会话中应使用的在线功能和连接方式 */
UENUM(BlueprintType)
enum class EOnlineSessionOnlineMode : uint8
{
	Offline,
	LAN,
	Online
};

/** 一个请求对象，用于存储创建游戏会话时所使用的参数 */
UCLASS(MinimalAPI, BlueprintType)
class UOnlineSession_HostSessionRequest : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category=Session)
	EOnlineSessionOnlineMode OnlineMode; //代表会话的类型,在线,离线,局域网
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUseLobbies; //是否使用大厅
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUseLobbiesVoiceChat; //是否使用大厅语音聊天
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUsePresence; //是否使用用户状态信息
	UPROPERTY(BlueprintReadWrite, Category=Session)
	FString ModeNameForAdvertisement; //匹配过程中用于指定这是哪种游戏模式的字符串
	UPROPERTY(BlueprintReadWrite, Category=Session, meta=(AllowedTypes="World"))
	FPrimaryAssetId MapID; //游戏开始时加载的地图，这需要是一个有效的主要资产顶级地图
	UPROPERTY(BlueprintReadWrite, Category=Session)
	TMap<FString, FString> ExtraArgs; //额外的参数作为URL选项传递给游戏
	UPROPERTY(BlueprintReadWrite, Category=Session)
	int32 MaxPlayerCount = 16; //每个游戏会话允许的最大玩家数 

	// ~ Begin Functions
	UE_API virtual int32 GetMaxPlayers() const { return MaxPlayerCount; } //获取实际应该使用的最大玩家数
	UE_API virtual FString GetMapName() const; //获取将在游戏过程中使用的完整地图名称
	UE_API virtual FString ConstructTravelURL() const; //构造将传递给ServerTravel的完整URL
	UE_API virtual bool ValidateAndLogErrors(FText& OutError) const; //如果此请求有效，则返回true，否则返回false并记录错误
	// ~ End Functions
};

/*从在线系统返回的结果对象，包含查询到的游戏会话参数,使用oss版本作函数区分*/
UCLASS(MinimalAPI, BlueprintType)
class UOnlineSession_SearchResult : public UObject
{
	GENERATED_BODY()

public:
	// ~ Begin Functions
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API FString GetDescription() const; /* 返回会话的内部描述，不便于人类阅读 */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API void GetStringSetting(FName Key, FString& OutValue, bool& bOutFoundValue) const; /* 获取任意字符串设置，如果该设置不存在，bFoundValue将为false */
	UFUNCTION(BlueprintPure, Category = Sessions)
	UE_API void GetIntSetting(FName Key, int32& OutValue, bool& bOutFoundValue) const; /* 获取任意整数设置，如果该设置不存在，bFoundValue将为false */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API int32 GetNumOpenPrivateConnections() const; /* 可用的专用连接数 */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API int32 GetNumOpenPublicConnections() const; /* 可用的公开连接数 */
	UFUNCTION(BlueprintPure, Category = Sessions)
	UE_API int32 GetMaxPublicConnections() const; /* 可用的最大公开连接数，包括已填充的连接 */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API int32 GetPingInMs() const; /* Ping到搜索结果，无法访问 MAX_QUERY_Ping */
	// ~ End Functions

	/** 指向不同oss版本的特定平台的指针 */
#if ONLINEUSER_OSSV1
	FOnlineSessionSearchResult Result;
#endif

#if !ONLINEUSER_OSSV1
	TSharedPtr<const UE::Online::FLobby> Lobby;
	UE::Online::FOnlineSessionId SessionID;
#endif
};

/** 会话搜索完成后，委托会被调用 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnlineSession_FindSessionsFinished, bool bSucceeded, const FText& ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineSession_FindSessionsFinishedDynamic, bool, bSucceeded, FText, ErrorMessage);

/** 描述会话搜索的请求对象，搜索完成后将更新此对象 */
UCLASS(MinimalAPI, BlueprintType)
class UOnlineSession_SearchSessionRequest : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = Session)
	EOnlineSessionOnlineMode OnlineMode; //指示这是在寻找完整的在线游戏还是其他类型的游戏，如局域网
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUseLobbies; //如果此请求应查找玩家托管的大厅（如果可用），则为True，如果为false，则仅搜索已注册的服务器会话
	UPROPERTY(BlueprintReadOnly, Category=Session)
	TArray<TObjectPtr<UOnlineSession_SearchResult>> Results; //调用OnSearchFinished时，所有已找到会话的列表将有效
	FOnlineSession_FindSessionsFinished OnSearchFinished; //会话搜索完成时调用本机代表
	UE_API void NotifySearchFinished(bool bSucceeded, const FText& ErrorMessage); //由子系统调用以执行完成的委托
private:
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Search Finished", AllowPrivateAccess = true))
	FOnlineSession_FindSessionsFinishedDynamic K2_OnSearchFinished; //会话搜索完成后，广播委托
};

/*UOnlineSessionSubsystem 使用的委托*/

/**
 * 当本地用户请求从外部源（例如从平台覆盖）加入会话时触发的事件。
 * 一般来说，游戏应该让玩家进入会话。
 * @param LocalPlatformUserId 接受邀请的本地用户id。这是一个平台用户id，因为该用户可能尚未登录。
 * @param RequestedSession 请求的会话。如果处理请求时出错，则可以为null。
 * @param RequestedSession 请求会话处理的结果
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnlineSessionOnUserRequestedSession, const FPlatformUserId& /*LocalPlatformUserId*/, UOnlineSession_SearchResult* /*RequestedSession*/, const FOnlineResultInformation& /*RequestedSessionResult*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineSessionOnUserRequestedSession_Dynamic, const FPlatformUserId&, LocalPlatformUserId, UOnlineSession_SearchResult*, RequestedSession, const FOnlineResultInformation&,
                                               RequestedSessionResult);

/**
 * 当会话加入完成时触发的事件，在加入基础会话之后，如果成功则在前往服务器之前触发。
 * 事件参数指示此操作是否成功，或者是否存在将阻止其运行的错误。
 * @param Result 会话加入的结果
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnlineSessionOnJoinSessionComplete, const FOnlineResultInformation& /*Result*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlineSessionOnJoinSessionComplete_Dynamic, const FOnlineResultInformation&, Result);

/**
 * 当托管会话创建完成时触发的事件，就在它到达地图之前。
 * 事件参数指示此操作是否成功，或者是否存在将阻止其运行的错误。
 * @param Result 会话加入的结果
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnlineSessionOnCreateSessionComplete, const FOnlineResultInformation& /*Result*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlineSessionOnCreateSessionComplete_Dynamic, const FOnlineResultInformation&, Result);

/**
 * 当本地用户请求从外部源（例如平台覆盖）销毁会话时触发的事件。
 * 游戏应该让玩家退出会话。
 * @param LocalPlatformUserId 发出销毁请求的本地用户id。这是一个平台用户id，因为该用户可能尚未登录。
 * @param SessionName 会话的名称标识符。
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnlineSessionOnDestroySessionRequested, const FPlatformUserId& /*LocalPlatformUserId*/, const FName& /*SessionName*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineSessionOnDestroySessionRequested_Dynamic, const FPlatformUserId&, LocalPlatformUserId, const FName&, SessionName);

/**
 * 在解析连接字符串之后和客户端移动之前，会话加入完成时触发的事件。
 * @param URL 解析了会话的连接字符串以及任何其他参数
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnlineSessionOnPreClientTravel, FString& /*URL*/);

/**
 * 在会话生态系统中的不同点触发的事件，表示会话的用户可呈现状态。
 * 这不应用于在线功能（对于这些功能，请使用OnCreateSessionComplete或OnJoinSessionComplete），而是用于丰富的在线状态等功能
 */
UENUM(BlueprintType)
enum class EOnlineSessionInformationState : uint8
{
	OutOfGame,
	Matchmaking,
	InGame
};

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnlineSessionOnSessionInformationChanged, EOnlineSessionInformationState /*SessionStatus*/, const FString& /*GameMode*/, const FString& /*MapName*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineSessionOnSessionInformationChanged_Dynamic, EOnlineSessionInformationState, SessionStatus, const FString&, GameMode, const FString&, MapName);

class FOnlineOnlineSearchSettingsBase : public FGCObject
{
public:
	FOnlineOnlineSearchSettingsBase(UOnlineSession_SearchSessionRequest* InSearchRequest)
	{
		SearchRequest = InSearchRequest;
	}

	virtual ~FOnlineOnlineSearchSettingsBase()
	{
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObject(SearchRequest);
	}

	virtual FString GetReferencerName() const override
	{
		static const FString NameString = TEXT("FOnlineOnlineSearchSettings");
		return NameString;
	}

public:
	TObjectPtr<UOnlineSession_SearchSessionRequest> SearchRequest = nullptr;
};

#if ONLINEUSER_OSSV1
class FOnlineSession_OnlineSessionSettings : public FOnlineSessionSettings
{
public:
	FOnlineSession_OnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4)
	{
		NumPublicConnections = MaxNumPlayers;
		if (NumPublicConnections < 0)
		{
			NumPublicConnections = 0;
		}
		NumPrivateConnections = 0;
		bIsLANMatch = bIsLAN;
		bShouldAdvertise = true;
		bAllowJoinInProgress = true;
		bAllowInvites = true;
		bUsesPresence = bIsPresence;
		bAllowJoinViaPresence = true;
		bAllowJoinViaPresenceFriendsOnly = false;
	}

	virtual ~FOnlineSession_OnlineSessionSettings()
	{
	}
};

class FOnlineOnlineSearchSettingsOSSv1 : public FOnlineSessionSearch, public FOnlineOnlineSearchSettingsBase
{
public:
	FOnlineOnlineSearchSettingsOSSv1(UOnlineSession_SearchSessionRequest* InSearchRequest)
		: FOnlineOnlineSearchSettingsBase(InSearchRequest)
	{
		bIsLanQuery = (InSearchRequest->OnlineMode == EOnlineSessionOnlineMode::LAN);
		MaxSearchResults = 10;
		PingBucketSize = 50;

		QuerySettings.Set(SETTING_ONLINESUBSYSTEM_VERSION, true, EOnlineComparisonOp::Equals);

		if (InSearchRequest->bUseLobbies)
		{
			QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
		}
	}

	virtual ~FOnlineOnlineSearchSettingsOSSv1()
	{
	}
};
#endif

#if !ONLINEUSER_OSSV1
class FOnlineOnlineSearchSettingsOSSv2 : public FOnlineOnlineSearchSettingsBase
{
public:
	FOnlineOnlineSearchSettingsOSSv2(UOnlineSession_SearchSessionRequest* InSearchRequest)
		: FOnlineOnlineSearchSettingsBase(InSearchRequest)
	{
		FindLobbyParams.MaxResults = 10;

		FindLobbyParams.Filters.Emplace(UE::Online::FFindLobbySearchFilter{SETTING_ONLINESUBSYSTEM_VERSION, UE::Online::ESchemaAttributeComparisonOp::Equals, true});
	}

public:
	UE::Online::FFindLobbies::Params FindLobbyParams;
};
#endif

/*
 * 托管和加入在线游戏的请求
 */
UCLASS(MinimalAPI, BlueprintType, Config=Engine)
class UOnlineSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ~ Begin UGameInstanceSubsystem Interface
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	// ~ End UGameInstanceSubsystem Interface

	// ~ UOnlineSessionSubsystem Interface
	UFUNCTION(BlueprintCallable, Category = Session)
	UE_API virtual UOnlineSession_HostSessionRequest* CreateOnlineHostSessionRequest(); //默认构造主机的Host对象，用于创建会话
	UFUNCTION(BlueprintCallable, Category = Session)
	UE_API virtual UOnlineSession_SearchSessionRequest* CreateOnlineSearchSessionRequest(); //默认构造查找对象，用于搜索会话
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API virtual void HostSession(APlayerController* HostingPlayer, UOnlineSession_HostSessionRequest* Request); //使用会话请求信息创建新的在线游戏，如果成功，将开始硬地图传输
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API virtual void QuickPlaySession(APlayerController* JoiningOrHostingPlayer, UOnlineSession_HostSessionRequest* HostRequest); //启动一个过程来查找现有会话，或者在没有找到可行会话的情况下创建一个新会话
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API virtual void JoinSession(APlayerController* JoiningPlayer, UOnlineSession_SearchResult* Request); //启动进程以加入现有会话，如果成功，将连接到指定的服务器
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API virtual void FindSessions(APlayerController* SearchingPlayer, UOnlineSession_SearchSessionRequest* Request); //在线系统查询与搜索请求匹配的可加入会话列表
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API virtual void CleanUpSessions(); //清理所有从返回主菜单等情况调用的活动会话
	// ~ UOnlineSessionSubsystem Interface

	// ~ Begin Events
	FOnlineSessionOnUserRequestedSession OnUserRequestedSessionEvent; //本地用户接受邀请时的本地代表
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On User Requested Session"))
	FOnlineSessionOnUserRequestedSession_Dynamic K2_OnUserRequestedSessionEvent; //本地用户接受邀请时的事件广播

	FOnlineSessionOnJoinSessionComplete OnJoinSessionCompleteEvent; //JoinSession调用完成时的本机代表
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Join Session Complete"))
	FOnlineSessionOnJoinSessionComplete_Dynamic K2_OnJoinSessionCompleteEvent; //JoinSession调用完成时的事件广播

	FOnlineSessionOnCreateSessionComplete OnCreateSessionCompleteEvent; //CreateSession调用完成时的本机代理
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Create Session Complete"))
	FOnlineSessionOnCreateSessionComplete_Dynamic K2_OnCreateSessionCompleteEvent; //CreateSession调用完成时的事件广播

	FOnlineSessionOnSessionInformationChanged OnSessionInformationChangedEvent; //当可呈现的会话信息发生变化时，本机代表
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Session Information Changed"))
	FOnlineSessionOnSessionInformationChanged_Dynamic K2_OnSessionInformationChangedEvent; //当可呈现的会话信息发生变化时，事件广播

	FOnlineSessionOnDestroySessionRequested OnDestroySessionRequestedEvent; //当请求销毁平台会话时，本机代表
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Leave Session Requested"))
	FOnlineSessionOnDestroySessionRequested_Dynamic K2_OnDestroySessionRequestedEvent; //当请求销毁平台会话时，事件广播

	FOnlineSessionOnPreClientTravel OnPreClientTravelEvent; //用于在客户旅行前修改连接URL的本地代表
	// ~ End Events

	//配置设置，这些可以在子类或配置文件中重写
	UPROPERTY(Config)
	bool bUseLobbiesDefault = true; //为会话搜索和主机请求设置bUseLobbies的默认值
	UPROPERTY(Config)
	bool bUseLobbiesVoiceChatDefault = false; //为会话主机请求设置bUseLobbiesVoiceChat的默认值
	UPROPERTY(Config)
	bool bUseBeacons = true; //在创建或加入游戏会话时，在服务器移动之前启用预订信标流

protected:
	// 在创建或加入会话的过程中调用的函数，这些函数可能会因游戏特定的行为而被覆盖

	/** 被调用以填写来自快速游戏主机设置的会话请求，可以针对特定于游戏的行为进行覆盖 */
	UE_API virtual TSharedRef<FOnlineOnlineSearchSettings> CreateQuickPlaySearchSettings(UOnlineSession_HostSessionRequest* Request, UOnlineSession_SearchSessionRequest* QuickPlayRequest);

	/** 当快速游戏搜索完成时调用，可以覆盖特定于游戏的行为 */
	UE_API virtual void HandleQuickPlaySearchFinished(bool bSucceeded, const FText& ErrorMessage, TWeakObjectPtr<APlayerController> JoiningOrHostingPlayer, TStrongObjectPtr<UOnlineSession_HostSessionRequest> HostRequest);

	/** 当前往会议失败时调用 */
	UE_API virtual void TravelLocalSessionFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ReasonString);

	/** 当创建新会话或创建失败时调用 */
	UE_API virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/** 调用以完成会话创建 */
	UE_API virtual void FinishSessionCreation(bool bWasSuccessful);

	/** 在前往新的托管会话地图后调用 */
	UE_API virtual void HandlePostLoadMap(UWorld* World);

protected:
	// 初始化和处理在线系统结果的内部功能

	UE_API void BindOnlineDelegates();
	UE_API void CreateOnlineSessionInternal(ULocalPlayer* LocalPlayer, UOnlineSession_HostSessionRequest* Request);
	UE_API void FindSessionsInternal(APlayerController* SearchingPlayer, const TSharedRef<FOnlineOnlineSearchSettings>& InSearchSettings);
	UE_API void JoinSessionInternal(ULocalPlayer* LocalPlayer, UOnlineSession_SearchResult* Request);
	UE_API void InternalTravelToSession(const FName SessionName);
	UE_API void NotifyUserRequestedSession(const FPlatformUserId& PlatformUserId, UOnlineSession_SearchResult* RequestedSession, const FOnlineResultInformation& RequestedSessionResult);
	UE_API void NotifyJoinSessionComplete(const FOnlineResultInformation& Result);
	UE_API void NotifyCreateSessionComplete(const FOnlineResultInformation& Result);
	UE_API void NotifySessionInformationUpdated(EOnlineSessionInformationState SessionStatusStr, const FString& GameMode = FString(), const FString& MapName = FString());
	UE_API void NotifyDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName);
	UE_API void SetCreateSessionError(const FText& ErrorText);

#if ONLINEUSER_OSSV1
	UE_API void BindOnlineDelegatesOSSv1();
	UE_API void CreateOnlineSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UOnlineSession_HostSessionRequest* Request);
	UE_API void FindSessionsInternalOSSv1(ULocalPlayer* LocalPlayer);
	UE_API void JoinSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UOnlineSession_SearchResult* Request);
	UE_API TSharedRef<FOnlineOnlineSearchSettings> CreateQuickPlaySearchSettingsOSSv1(UOnlineSession_HostSessionRequest* Request, UOnlineSession_SearchSessionRequest* QuickPlayRequest);
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

#if !ONLINEUSER_OSSV1
	UE_API void BindOnlineDelegatesOSSv2();
	UE_API void CreateOnlineSessionInternalOSSv2(ULocalPlayer* LocalPlayer, UOnlineSession_HostSessionRequest* Request);
	UE_API void FindSessionsInternalOSSv2(ULocalPlayer* LocalPlayer);
	UE_API void JoinSessionInternalOSSv2(ULocalPlayer* LocalPlayer, UOnlineSession_SearchResult* Request);
	UE_API TSharedRef<FOnlineOnlineSearchSettings> CreateQuickPlaySearchSettingsOSSv2(UOnlineSession_HostSessionRequest* HostRequest, UOnlineSession_SearchSessionRequest* SearchRequest);
	UE_API void CleanUpSessionsOSSv2();
	UE_API void OnLobbyJoinRequested(const UE::Online::FUILobbyJoinRequested& EventParams); //处理来自在线服务的加入请求
	UE_API void OnSessionJoinRequested(const UE::Online::FUISessionJoinRequested& EventParams); //处理来自在线服务的会话加入请求
	UE_API UE::Online::FAccountId GetAccountId(APlayerController* PlayerController) const; //获取给定控制器的本地用户id
	UE_API UE::Online::FLobbyId GetLobbyId(const FName SessionName) const; //获取给定会话名称的大厅id
	UE::Online::FOnlineEventDelegateHandle LobbyJoinRequestedHandle; //请求UI大厅加入的事件句柄
	UE::Online::FOnlineEventDelegateHandle SessionJoinRequestedHandle; //请求UI大厅会话的事件句柄
#endif

	UE_API void CreateHostReservationBeacon();
	UE_API void ConnectToHostReservationBeacon();
	UE_API void DestroyHostReservationBeacon();

protected:
	FString PendingTravelURL; //会话操作完成后将使用的旅行URL
	FOnlineResultInformation CreateSessionResult; //会话创建尝试的最新结果信息存储在此处，以便以后存储错误代码
	bool bWantToDestroyPendingSession = false; //如果我们想在会话创建后取消会话，则为True
	bool bIsDedicatedServer = false; //如果这是一个专用服务器，不需要LocalPlayer创建会话，则为True
	TSharedPtr<FOnlineOnlineSearchSettings> SearchSettings; //当前搜索的设置

	UPROPERTY(Transient)
	TWeakObjectPtr<AOnlineBeaconHost> BeaconHostListener; //用于注册信标的通用信标监听器

	UPROPERTY(Transient)
	TObjectPtr<UPartyBeaconState> ReservationBeaconHostState; //信标主机的状态

	UPROPERTY(Transient)
	TWeakObjectPtr<APartyBeaconHost> ReservationBeaconHost; //控制此游戏访问的信标

	UPROPERTY(Transient)
	TWeakObjectPtr<APartyBeaconClient> ReservationBeaconClient; //信标通信的在线类对象

	UPROPERTY(Config)
	int32 BeaconTeamCount = 2; //信标预约团队数量
	UPROPERTY(Config)
	int32 BeaconTeamSize = 8; //信标预订团队的规模
	UPROPERTY(Config)
	int32 BeaconMaxReservations = 16; //信标预留的最大数量
};

#undef UE_API
