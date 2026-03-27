// Copyright © 2026 鸿源z. All Rights Reserved.


#include "OnlineSessionSubsystem.h"

#include "PartyBeaconClient.h"
#include "Engine/AssetManager.h"
#include "Online/OnlineSessionNames.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include "OnlineBeaconHost.h"
#include "OnlineSessionSettings.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Interfaces/OnlineSessionInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OnlineSessionSubsystem)

#if ONLINEUSER_OSSV1
#include "Engine/World.h"
#include "OnlineSubsystemUtils.h"
#endif

#if !ONLINEUSER_OSSV1
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Online/OnlineServicesEngineUtils.h"
#include "OnlineSubsystemUtils.h"
using namespace UE::Online;
#endif

#define LOCTEXT_NAMESPACE "OnlineUser"

// ~ Begin UOnlineSession_HostSessionRequest
FString UOnlineSession_HostSessionRequest::GetMapName() const
{
	FAssetData MapAssetData;
	if (UAssetManager::Get().GetPrimaryAssetData(MapID, /*out*/ MapAssetData))
		return MapAssetData.PackageName.ToString();

	return FString();
}

FString UOnlineSession_HostSessionRequest::ConstructTravelURL() const
{
	FString CombinedExtraArgs;

	if (OnlineMode == EOnlineSessionOnlineMode::LAN)
		CombinedExtraArgs += TEXT("?bIsLanMatch");

	if (OnlineMode != EOnlineSessionOnlineMode::Offline)
		CombinedExtraArgs += TEXT("?listen");

	for (const auto& KVP : ExtraArgs)
	{
		if (!KVP.Key.IsEmpty())
		{
			if (KVP.Value.IsEmpty())
			{
				CombinedExtraArgs += FString::Printf(TEXT("?%s"), *KVP.Key);
			}
			else
			{
				CombinedExtraArgs += FString::Printf(TEXT("?%s=%s"), *KVP.Key, *KVP.Value);
			}
		}
	}

	//bIsRecordingDemo ? TEXT("?DemoRec") : TEXT(""));

	return FString::Printf(TEXT("%s%s"),
	                       *GetMapName(),
	                       *CombinedExtraArgs);
}

bool UOnlineSession_HostSessionRequest::ValidateAndLogErrors(FText& OutError) const
{
#if WITH_SERVER_CODE
	if (GetMapName().IsEmpty())
	{
		OutError = FText::Format(
			NSLOCTEXT("NetworkErrors", "InvalidMapFormat", "Can't find asset data for MapID {0}, hosting request failed."), FText::FromString(MapID.ToString()));
		return false;
	}

	return true;
#else
	// Client builds are only meant to connect to dedicated servers, they are missing the code to host a session by default
	// You can change this behavior in subclasses to handle something like a tutorial
	OutError = NSLOCTEXT("NetworkErrors", "ClientBuildCannotHost", "Client builds cannot host game sessions.");
	return false;
#endif
}

// ~ Begin UOnlineSession_SearchResult
#if ONLINEUSER_OSSV1
FString UOnlineSession_SearchResult::GetDescription() const
{
	return Result.GetSessionIdStr();
}

void UOnlineSession_SearchResult::GetStringSetting(FName Key, FString& OutValue, bool& bOutFoundValue) const
{
	bOutFoundValue = Result.Session.SessionSettings.Get<FString>(Key, /*out*/ OutValue);
}

void UOnlineSession_SearchResult::GetIntSetting(FName Key, int32& OutValue, bool& bOutFoundValue) const
{
	bOutFoundValue = Result.Session.SessionSettings.Get<int32>(Key, /*out*/ OutValue);
}

int32 UOnlineSession_SearchResult::GetNumOpenPrivateConnections() const
{
	return Result.Session.NumOpenPrivateConnections;
}

int32 UOnlineSession_SearchResult::GetNumOpenPublicConnections() const
{
	return Result.Session.NumOpenPublicConnections;
}

int32 UOnlineSession_SearchResult::GetMaxPublicConnections() const
{
	return Result.Session.SessionSettings.NumPublicConnections;
}

int32 UOnlineSession_SearchResult::GetPingInMs() const
{
	return Result.PingInMs;
}
#endif

#if !ONLINEUSER_OSSV1
FString UOnlineSession_SearchResult::GetDescription() const
{
	return ToLogString(Lobby->LobbyId);
}

void UOnlineSession_SearchResult::GetStringSetting(FName Key, FString& OutValue, bool& bOutFoundValue) const
{
	if (const FSchemaVariant* VariantValue = Lobby->Attributes.Find(Key))
	{
		bOutFoundValue = true;
		OutValue = VariantValue->GetString();
	}
	else
	{
		bOutFoundValue = false;
	}
}

void UOnlineSession_SearchResult::GetIntSetting(FName Key, int32& OutValue, bool& bOutFoundValue) const
{
	if (const FSchemaVariant* VariantValue = Lobby->Attributes.Find(Key))
	{
		bOutFoundValue = true;
		OutValue = (int32)VariantValue->GetInt64();
	}
	else
	{
		bOutFoundValue = false;
	}
}

int32 UOnlineSession_SearchResult::GetNumOpenPrivateConnections() const
{
	// TODO:  私人链接
	return 0;
}

int32 UOnlineSession_SearchResult::GetNumOpenPublicConnections() const
{
	return Lobby->MaxMembers - Lobby->Members.Num();
}

int32 UOnlineSession_SearchResult::GetMaxPublicConnections() const
{
	return Lobby->MaxMembers;
}

int32 UOnlineSession_SearchResult::GetPingInMs() const
{
	// TODO:  获取网络延迟（Ping）这个功能，目前不是游戏大厅（Lobbies）的属性。需要在使用游戏会话（Sessions）时再实现
	return 0;
}
#endif


// ~ Begin UOnlineSession_SearchSessionRequest
void UOnlineSession_SearchSessionRequest::NotifySearchFinished(bool bSucceeded, const FText& ErrorMessage)
{
	OnSearchFinished.Broadcast(bSucceeded, ErrorMessage);
	K2_OnSearchFinished.Broadcast(bSucceeded, ErrorMessage);
}


// ~ Begin UOnlineSessionSubsystem
void UOnlineSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BindOnlineDelegates();
	GEngine->OnTravelFailure().AddUObject(this, &ThisClass::TravelLocalSessionFailure);

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);

	UGameInstance* GameInstance = GetGameInstance();
	bIsDedicatedServer = GameInstance->IsDedicatedServerInstance();
}

void UOnlineSessionSubsystem::Deinitialize()
{
#if ONLINEUSER_OSSV1
	if (IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld()))
	{
		// 在关机期间，这可能无效
		const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface) SessionInterface->ClearOnSessionFailureDelegates(this);
	}
#endif

	if (GEngine)
	{
		GEngine->OnTravelFailure().RemoveAll(this);
	}

	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	Super::Deinitialize();
}

bool UOnlineSessionSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	//查询子类,用继承链往下找
	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	// 仅在没有游戏特定子类的情况下创建实例
	return ChildClasses.Num() == 0;
}

UOnlineSession_HostSessionRequest* UOnlineSessionSubsystem::CreateOnlineHostSessionRequest()
{
	/**游戏特定的子系统可以覆盖此功能，也可以在创建后进行修改*/
	UOnlineSession_HostSessionRequest* NewRequest = NewObject<UOnlineSession_HostSessionRequest>(this);
	NewRequest->OnlineMode = EOnlineSessionOnlineMode::Online;
	NewRequest->bUseLobbies = bUseLobbiesDefault;
	NewRequest->bUseLobbiesVoiceChat = bUseLobbiesVoiceChatDefault;

	// 默认情况下，我们在用于配对的主会话中启用在线状态。对于关心在线状态的在线系统，只有主会话应该启用在线状态
	NewRequest->bUsePresence = !IsRunningDedicatedServer();

	return NewRequest;
}

UOnlineSession_SearchSessionRequest* UOnlineSessionSubsystem::CreateOnlineSearchSessionRequest()
{
	/**游戏特定的子系统可以覆盖此功能，也可以在创建后进行修改*/
	UOnlineSession_SearchSessionRequest* NewRequest = NewObject<UOnlineSession_SearchSessionRequest>(this);
	NewRequest->OnlineMode = EOnlineSessionOnlineMode::Online;

	NewRequest->bUseLobbies = bUseLobbiesDefault;

	return NewRequest;
}

void UOnlineSessionSubsystem::HostSession(APlayerController* HostingPlayer, UOnlineSession_HostSessionRequest* Request)
{
	if (Request == nullptr)
	{
		SetCreateSessionError(NSLOCTEXT("NetworkErrors", "InvalidRequest", "HostSession passed an invalid request."));
		OnCreateSessionComplete(NAME_None, false);
		return;
	}

	ULocalPlayer* LocalPlayer = (HostingPlayer != nullptr) ? HostingPlayer->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr && !bIsDedicatedServer)
	{
		SetCreateSessionError(NSLOCTEXT("NetworkErrors", "InvalidHostingPlayer", "HostingPlayer is invalid."));
		OnCreateSessionComplete(NAME_None, false);
		return;
	}

	FText OutError;
	if (!Request->ValidateAndLogErrors(OutError))
	{
		SetCreateSessionError(OutError);
		OnCreateSessionComplete(NAME_None, false);
		return;
	}

	if (Request->OnlineMode == EOnlineSessionOnlineMode::Offline)
	{
		if (GetWorld()->GetNetMode() == NM_Client)
		{
			SetCreateSessionError(NSLOCTEXT("NetworkErrors", "CannotHostAsClient", "Cannot host offline game as client."));
			OnCreateSessionComplete(NAME_None, false);
			return;
		}
		else
		{
			// Offline so travel to the specified match URL immediately
			GetWorld()->ServerTravel(Request->ConstructTravelURL());
		}
	}
	else
	{
		CreateOnlineSessionInternal(LocalPlayer, Request);
	}

	NotifySessionInformationUpdated(EOnlineSessionInformationState::InGame, Request->ModeNameForAdvertisement, Request->GetMapName());
}

void UOnlineSessionSubsystem::QuickPlaySession(APlayerController* JoiningOrHostingPlayer, UOnlineSession_HostSessionRequest* HostRequest)
{
	UE_LOG(LogOnlineSession, Log, TEXT("QuickPlay Requested"));

	if (HostRequest == nullptr)
	{
		UE_LOG(LogOnlineSession, Error, TEXT("QuickPlaySession passed a null request"));
		return;
	}

	TStrongObjectPtr<UOnlineSession_HostSessionRequest> HostRequestPtr = TStrongObjectPtr<UOnlineSession_HostSessionRequest>(HostRequest);
	TWeakObjectPtr<APlayerController> JoiningOrHostingPlayerPtr = TWeakObjectPtr<APlayerController>(JoiningOrHostingPlayer);

	UOnlineSession_SearchSessionRequest* QuickPlayRequest = CreateOnlineSearchSessionRequest();
	QuickPlayRequest->OnSearchFinished.AddUObject(this, &UOnlineSessionSubsystem::HandleQuickPlaySearchFinished, JoiningOrHostingPlayerPtr, HostRequestPtr);

	// 默认情况下，我们在用于配对的主会话上启用在线状态。对于关心在线状态的在线系统，只有主会话应该启用在线状态

	HostRequestPtr->bUseLobbies = bUseLobbiesDefault;
	HostRequestPtr->bUseLobbiesVoiceChat = bUseLobbiesVoiceChatDefault;
	HostRequestPtr->bUsePresence = true;
	QuickPlayRequest->bUseLobbies = bUseLobbiesDefault;

	NotifySessionInformationUpdated(EOnlineSessionInformationState::Matchmaking);
	FindSessionsInternal(JoiningOrHostingPlayer, CreateQuickPlaySearchSettings(HostRequest, QuickPlayRequest));
}

void UOnlineSessionSubsystem::JoinSession(APlayerController* JoiningPlayer, UOnlineSession_SearchResult* Request)
{
	if (Request == nullptr)
	{
		UE_LOG(LogOnlineSession, Error, TEXT("JoinSession passed a null request"));
		return;
	}

	ULocalPlayer* LocalPlayer = (JoiningPlayer != nullptr) ? JoiningPlayer->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogOnlineSession, Error, TEXT("JoiningPlayer is invalid"));
		return;
	}

	//更新此处的状态，因为客户旅行后我们将没有原始游戏模式和地图名称密钥。如果加入/旅行失败，则重置为主菜单
	FString SessionGameMode, SessionMapName;
	bool bEmpty;
#if ONLINEUSER_OSSV1
	Request->GetStringSetting(SETTING_GAMEMODE, SessionGameMode, bEmpty);
	Request->GetStringSetting(SETTING_MAPNAME, SessionMapName, bEmpty);
	NotifySessionInformationUpdated(EOnlineSessionInformationState::InGame, SessionGameMode, SessionMapName);
#else
	if (Request->Lobby.IsValid())
	{
		Request->GetStringSetting(SETTING_GAMEMODE, SessionGameMode, bEmpty);
		Request->GetStringSetting(SETTING_MAPNAME, SessionMapName, bEmpty);
		NotifySessionInformationUpdated(EOnlineSessionInformationState::InGame, SessionGameMode, SessionMapName);
	}
#endif

	JoinSessionInternal(LocalPlayer, Request);
}

void UOnlineSessionSubsystem::FindSessions(APlayerController* SearchingPlayer, UOnlineSession_SearchSessionRequest* Request)
{
	if (Request == nullptr)
	{
		UE_LOG(LogOnlineSession, Error, TEXT("FindSessions passed a null request"));
		return;
	}

#if ONLINEUSER_OSSV1
	FindSessionsInternal(SearchingPlayer, MakeShared<FOnlineOnlineSearchSettingsOSSv1>(Request));
#else
	FindSessionsInternal(SearchingPlayer, MakeShared<FOnlineOnlineSearchSettingsOSSv2>(Request));
#endif
}

void UOnlineSessionSubsystem::CleanUpSessions()
{
	bWantToDestroyPendingSession = true;

	if (bUseBeacons) DestroyHostReservationBeacon();


	NotifySessionInformationUpdated(EOnlineSessionInformationState::OutOfGame);
#if ONLINEUSER_OSSV1
	CleanUpSessionsOSSv1();
#else
	CleanUpSessionsOSSv2();
#endif
}

void UOnlineSessionSubsystem::CreateHostReservationBeacon()
{
	check(!BeaconHostListener.IsValid());
	check(!ReservationBeaconHost.IsValid());

	UWorld* const World = GetWorld();
	BeaconHostListener = World->SpawnActor<AOnlineBeaconHost>(AOnlineBeaconHost::StaticClass());
	check(BeaconHostListener.IsValid());
	verify(BeaconHostListener->InitHost());

	ReservationBeaconHost = World->SpawnActor<APartyBeaconHost>(APartyBeaconHost::StaticClass());
	check(ReservationBeaconHost.IsValid());

	if (ReservationBeaconHostState)
	{
		ReservationBeaconHost->InitFromBeaconState(&*ReservationBeaconHostState);
	}
	else
	{
		// TODO: 我们现在使用默认的硬编码值作为参数，但它们是可配置的
		ReservationBeaconHost->InitHostBeacon(BeaconTeamCount, BeaconTeamSize, BeaconMaxReservations, NAME_GameSession);
		ReservationBeaconHostState = ReservationBeaconHost->GetState();
	}

	BeaconHostListener->RegisterHost(ReservationBeaconHost.Get());
	BeaconHostListener->PauseBeaconRequests(false);
}

void UOnlineSessionSubsystem::ConnectToHostReservationBeacon()
{
	UWorld* const World = GetWorld();
	check(World);
	ReservationBeaconClient = World->SpawnActor<APartyBeaconClient>(APartyBeaconClient::StaticClass());
	check(ReservationBeaconClient.IsValid());

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(World);
	check(OnlineSub);
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions);
	FNamedOnlineSession* Session = Sessions->GetNamedSession(NAME_GameSession);
	check(Session);
	FString SessionIdStr = Session->GetSessionIdStr();

	FString ConnectInfo;
	Sessions->GetResolvedConnectString(NAME_GameSession, ConnectInfo, NAME_BeaconPort);

	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	check(Identity);
	FUniqueNetIdWrapper DefaultNetId = Identity->GetUniquePlayerId(0);
	check(DefaultNetId.IsValid());

	FPlayerReservation PlayerReservation;
	PlayerReservation.UniqueId = *DefaultNetId;
	PlayerReservation.Platform = OnlineSub->GetLocalPlatformName();

	ReservationBeaconClient->OnHostConnectionFailure().BindWeakLambda(this, [this]()
	{
		// We only want to react to failure calls while the connection is active, not when it closes
		if (ReservationBeaconClient->GetNetDriver())
		{
			FOnlineResultInformation JoinSessionResult;
			JoinSessionResult.bWasSuccessful = false;
			JoinSessionResult.ErrorId = TEXT("UnknownError");

			NotifyJoinSessionComplete(JoinSessionResult);
			NotifySessionInformationUpdated(EOnlineSessionInformationState::OutOfGame);

			CleanUpSessions();
		}
	});

	ReservationBeaconClient->OnReservationRequestComplete().BindWeakLambda(this, [this](EPartyReservationResult::Type ReservationResponse)
	{
		if (ReservationResponse == EPartyReservationResult::ReservationAccepted || ReservationResponse == EPartyReservationResult::ReservationDuplicate)
		{
			FOnlineResultInformation JoinSessionResult;
			JoinSessionResult.bWasSuccessful = true;
			NotifyJoinSessionComplete(JoinSessionResult);

			InternalTravelToSession(NAME_GameSession);
		}
		else
		{
			FOnlineResultInformation JoinSessionResult;
			JoinSessionResult.bWasSuccessful = false;
			JoinSessionResult.ErrorId = TEXT("UnknownError");

			NotifyJoinSessionComplete(JoinSessionResult);
			NotifySessionInformationUpdated(EOnlineSessionInformationState::OutOfGame);

			CleanUpSessions();
		}
	});

	ReservationBeaconClient->RequestReservation(ConnectInfo, SessionIdStr, *DefaultNetId, {PlayerReservation});
}

void UOnlineSessionSubsystem::DestroyHostReservationBeacon()
{
	if (BeaconHostListener.IsValid() && ReservationBeaconHost.IsValid())
	{
		BeaconHostListener->UnregisterHost(ReservationBeaconHost->GetBeaconType());
	}
	if (BeaconHostListener.IsValid())
	{
		BeaconHostListener->Destroy();
		BeaconHostListener = nullptr;
	}
	if (ReservationBeaconHost.IsValid())
	{
		ReservationBeaconHost->Destroy();
		ReservationBeaconHost = nullptr;
	}
}

TSharedRef<FOnlineOnlineSearchSettings> UOnlineSessionSubsystem::CreateQuickPlaySearchSettings(UOnlineSession_HostSessionRequest* HostRequest, UOnlineSession_SearchSessionRequest* SearchRequest)
{
#if ONLINEUSER_OSSV1
	return CreateQuickPlaySearchSettingsOSSv1(HostRequest, SearchRequest);
#else
	return CreateQuickPlaySearchSettingsOSSv2(HostRequest, SearchRequest);
#endif // OnlineUSER_OSSV1
}

void UOnlineSessionSubsystem::HandleQuickPlaySearchFinished(bool bSucceeded, const FText& ErrorMessage, TWeakObjectPtr<APlayerController> JoiningOrHostingPlayer, TStrongObjectPtr<UOnlineSession_HostSessionRequest> HostRequest)
{
	const int32 ResultCount = SearchSettings->SearchRequest->Results.Num();
	UE_LOG(LogOnlineSession, Log, TEXT("QuickPlay Search Finished %s (Results %d) (Error: %s)"), bSucceeded ? TEXT("Success") : TEXT("Failed"), ResultCount, *ErrorMessage.ToString());

	//@TODO:我们必须检查错误消息是否为空，因为一些OSS层仅仅因为没有会话就报告了故障。请使用OSS 2.0进行修复。
	if (bSucceeded || ErrorMessage.IsEmpty())
	{
		// 加入最佳搜索结果。
		if (ResultCount > 0)
		{
			//@TODO: 我们也许应该看看ping？也许还有其他因素可以找到最好的。我知道它们是否经过预先分类。
			for (UOnlineSession_SearchResult* Result : SearchSettings->SearchRequest->Results)
			{
				JoinSession(JoiningOrHostingPlayer.Get(), Result);
				return;
			}
		}
		else
		{
			HostSession(JoiningOrHostingPlayer.Get(), HostRequest.Get());
		}
	}
	else
	{
		//@TODO: 这太糟糕了，需要告诉别人。
		NotifySessionInformationUpdated(EOnlineSessionInformationState::OutOfGame);
	}
}

void UOnlineSessionSubsystem::TravelLocalSessionFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ReasonString)
{
	// 这个委托是全局的，但PIE可以有多个游戏实例，所以make
	// 确保它是为与此游戏实例子系统关联的同一个世界而提出的
	if (World != GetWorld())
	{
		return;
	}

	UE_LOG(LogOnlineSession, Warning, TEXT("TravelLocalSessionFailure(World: %s, FailureType: %s, ReasonString: %s)"),
	       *GetPathNameSafe(World),
	       ETravelFailure::ToString(FailureType),
	       *ReasonString);

	// TODO: 广播这个失败的时候，我们也能够广播一个成功。目前，我们在开始旅行之前广播了一个成功，所以成功之后的失败令人困惑。
	//FOnlineResultInformation JoinSessionResult;
	//JoinSessionResult.bWasSuccessful = false;
	//JoinSessionResult.ErrorId = ReasonString; // TODO:  这是一个足够的ErrorId吗？
	//JoinSessionResult.ErrorText = FText::FromString(ReasonString);
	//NotifyJoinSessionComplete(JoinSessionResult);
	NotifySessionInformationUpdated(EOnlineSessionInformationState::OutOfGame);
}

void UOnlineSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnlineSession, Log, TEXT("OnCreateSessionComplete(SessionName: %s, bWasSuccessful: %d)"), *SessionName.ToString(), bWasSuccessful);

#if ONLINEUSER_OSSV1 // OSSv2加入分屏播放器作为创建调用的一部分
	// 添加分屏播放器（如果存在）
#if 0 //@TODO:
	if (bWasSuccessful && LocalPlayers.Num() > 1)
	{
		IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
		if (Sessions.IsValid() && LocalPlayers[1]->GetPreferredUniqueNetId().IsValid())
		{
			Sessions->RegisterLocalPlayer(*LocalPlayers[1]->GetPreferredUniqueNetId(), NAME_GameSession,
			                              FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &ThisClass::OnRegisterLocalPlayerComplete_CreateSession));
		}
	}
	else
#endif
#endif
	{
		// 我们要么失败了，要么只有一个本地用户
		FinishSessionCreation(bWasSuccessful);
	}
}

void UOnlineSessionSubsystem::FinishSessionCreation(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		//@TODO 将此时间与加入回调同步，如果计划发生变化，则修改这两个位置和注释
		CreateSessionResult = FOnlineResultInformation();
		CreateSessionResult.bWasSuccessful = true;

		if (bUseBeacons)
		{
			CreateHostReservationBeacon();
		}

		NotifyCreateSessionComplete(CreateSessionResult);

		// 前往指定的匹配URL
		GetWorld()->ServerTravel(PendingTravelURL);
	}
	else
	{
		if (CreateSessionResult.bWasSuccessful || CreateSessionResult.ErrorText.IsEmpty())
		{
			FString ReturnError = TEXT("GenericFailure"); // TODO: 无法从OSSV1中获取会话错误代码
			FText ReturnReason = NSLOCTEXT("NetworkErrors", "CreateSessionFailed", "Failed to create session.");

			CreateSessionResult.bWasSuccessful = false;
			CreateSessionResult.ErrorId = ReturnError;
			CreateSessionResult.ErrorText = ReturnReason;
		}

		UE_LOG(LogOnlineSession, Error, TEXT("FinishSessionCreation(%s): %s"), *CreateSessionResult.ErrorId, *CreateSessionResult.ErrorText.ToString());

		NotifyCreateSessionComplete(CreateSessionResult);
		NotifySessionInformationUpdated(EOnlineSessionInformationState::OutOfGame);
	}
}

void UOnlineSessionSubsystem::HandlePostLoadMap(UWorld* World)
{
	// 检测世界有效性
	if (!World) return;

	// 忽略任何不属于此游戏实例的世界，编辑器中可能会出现这种情况。
	if (World->GetGameInstance() != GetGameInstance()) return;


	// 除非世界类型是game/pie，否则我们不关心更新会话。
	if (!(World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE)) return;


#if ONLINEUSER_OSSV1
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	check(SessionInterface.IsValid());

	const FName SessionName(NAME_GameSession);
	FNamedOnlineSession* CurrentSession = SessionInterface->GetNamedSession(SessionName);

	// 如果我们正在主持会话，请更新广告中的地图名称。
	if (CurrentSession != nullptr && CurrentSession->bHosting)
	{
		// 这需要完整的包路径来匹配主机的GetMapName函数，World->GetMapName当前是短名称-更新主机设置
		CurrentSession->SessionSettings.Set(SETTING_MAPNAME, UWorld::RemovePIEPrefix(World->GetOutermost()->GetName()), EOnlineDataAdvertisementType::ViaOnlineService);

		SessionInterface->UpdateSession(SessionName, CurrentSession->SessionSettings, true);

		if (bUseBeacons) CreateHostReservationBeacon();
	}
#endif
}

void UOnlineSessionSubsystem::BindOnlineDelegates()
{
#if ONLINEUSER_OSSV1
	BindOnlineDelegatesOSSv1();
#else
	BindOnlineDelegatesOSSv2();
#endif
}

void UOnlineSessionSubsystem::CreateOnlineSessionInternal(ULocalPlayer* LocalPlayer, UOnlineSession_HostSessionRequest* Request)
{
	CreateSessionResult = FOnlineResultInformation();
	PendingTravelURL = Request->ConstructTravelURL();

#if ONLINEUSER_OSSV1
	CreateOnlineSessionInternalOSSv1(LocalPlayer, Request);
#else
	CreateOnlineSessionInternalOSSv2(LocalPlayer, Request);
#endif
}

void UOnlineSessionSubsystem::FindSessionsInternal(APlayerController* SearchingPlayer, const TSharedRef<FOnlineOnlineSearchSettings>& InSearchSettings)
{
	if (SearchSettings.IsValid())
	{
		//@TODO: This is a poor user experience for the API user, we should let the additional search piggyback and
		// just give it the same results as the currently pending one
		// (or enqueue the request and service it when the previous one finishes or fails)
		UE_LOG(LogOnlineSession, Error, TEXT("A previous FindSessions call is still in progress, aborting"));
		SearchSettings->SearchRequest->NotifySearchFinished(false, LOCTEXT("Error_FindSessionAlreadyInProgress", "Session search already in progress"));
	}

	ULocalPlayer* LocalPlayer = (SearchingPlayer != nullptr) ? SearchingPlayer->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogOnlineSession, Error, TEXT("SearchingPlayer is invalid"));
		InSearchSettings->SearchRequest->NotifySearchFinished(false, LOCTEXT("Error_FindSessionBadPlayer", "Session search was not provided a local player"));
		return;
	}

	SearchSettings = InSearchSettings;
#if ONLINEUSER_OSSV1
	FindSessionsInternalOSSv1(LocalPlayer);
#else
	FindSessionsInternalOSSv2(LocalPlayer);
#endif
}

void UOnlineSessionSubsystem::JoinSessionInternal(ULocalPlayer* LocalPlayer, UOnlineSession_SearchResult* Request)
{
#if ONLINEUSER_OSSV1
	JoinSessionInternalOSSv1(LocalPlayer, Request);
#else
	JoinSessionInternalOSSv2(LocalPlayer, Request);
#endif
}

void UOnlineSessionSubsystem::InternalTravelToSession(const FName SessionName)
{
	//@TODO: Ideally we'd use triggering player instead of first (they're all gonna go at once so it probably doesn't matter)
	APlayerController* const PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if (PlayerController == nullptr)
	{
		FText ReturnReason = NSLOCTEXT("NetworkErrors", "InvalidPlayerController", "Invalid Player Controller");
		UE_LOG(LogOnlineSession, Error, TEXT("InternalTravelToSession(Failed due to %s)"), *ReturnReason.ToString());
		return;
	}

	FString URL;
#if ONLINEUSER_OSSV1
	// travel to session
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions.IsValid());

	if (!Sessions->GetResolvedConnectString(SessionName, URL))
	{
		FText FailReason = NSLOCTEXT("NetworkErrors", "TravelSessionFailed", "Travel to Session failed.");
		UE_LOG(LogOnlineSession, Error, TEXT("InternalTravelToSession(%s)"), *FailReason.ToString());
		return;
	}
#else
	TSharedPtr<IOnlineServices> OnlineServices = GetServices(GetWorld(), EOnlineServices::Default);
	check(OnlineServices);

	FAccountId LocalUserId = GetAccountId(PlayerController);
	if (LocalUserId.IsValid())
	{
		TOnlineResult<FGetResolvedConnectString> Result;
		if (!bUseLobbiesDefault)
		{
			ISessionsPtr Sessions = OnlineServices->GetSessionsInterface();
			check(Sessions)
			TOnlineResult<FGetSessionByName> SessionResult = Sessions->GetSessionByName({SessionName});

			if (SessionResult.IsOk())
			{
				Result = OnlineServices->GetResolvedConnectString({LocalUserId, FLobbyId(), SessionResult.GetOkValue().Session->GetSessionId(), NAME_GamePort});
			}
		}
		else
		{
			Result = OnlineServices->GetResolvedConnectString({LocalUserId, GetLobbyId(SessionName)});
		}

		if (ensure(Result.IsOk()))
		{
			URL = Result.GetOkValue().ResolvedConnectString;
		}
	}
#endif

	// 允许在旅行前修改URL
	OnPreClientTravelEvent.Broadcast(URL);

	PlayerController->ClientTravel(URL, TRAVEL_Absolute);
}

void UOnlineSessionSubsystem::NotifyUserRequestedSession(const FPlatformUserId& PlatformUserId, UOnlineSession_SearchResult* RequestedSession, const FOnlineResultInformation& RequestedSessionResult)
{
	OnUserRequestedSessionEvent.Broadcast(PlatformUserId, RequestedSession, RequestedSessionResult);
	K2_OnUserRequestedSessionEvent.Broadcast(PlatformUserId, RequestedSession, RequestedSessionResult);
}

void UOnlineSessionSubsystem::NotifyJoinSessionComplete(const FOnlineResultInformation& Result)
{
	OnJoinSessionCompleteEvent.Broadcast(Result);
	K2_OnJoinSessionCompleteEvent.Broadcast(Result);
}

void UOnlineSessionSubsystem::NotifyCreateSessionComplete(const FOnlineResultInformation& Result)
{
	OnCreateSessionCompleteEvent.Broadcast(Result);
	K2_OnCreateSessionCompleteEvent.Broadcast(Result);
}

void UOnlineSessionSubsystem::NotifySessionInformationUpdated(EOnlineSessionInformationState SessionStatusStr, const FString& GameMode, const FString& MapName)
{
	OnSessionInformationChangedEvent.Broadcast(SessionStatusStr, GameMode, MapName);
	K2_OnSessionInformationChangedEvent.Broadcast(SessionStatusStr, GameMode, MapName);
}

void UOnlineSessionSubsystem::NotifyDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName)
{
	OnDestroySessionRequestedEvent.Broadcast(PlatformUserId, SessionName);
	K2_OnDestroySessionRequestedEvent.Broadcast(PlatformUserId, SessionName);
}

void UOnlineSessionSubsystem::SetCreateSessionError(const FText& ErrorText)
{
	CreateSessionResult.bWasSuccessful = false;
	CreateSessionResult.ErrorId = TEXT("InternalFailure");

	// TODO May want to replace with a generic error text in shipping builds depending on how much data you want to give users
	CreateSessionResult.ErrorText = ErrorText;
}

#if ONLINEUSER_OSSV1
void UOnlineSessionSubsystem::BindOnlineDelegatesOSSv1()
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	check(SessionInterface.IsValid());

	SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));
	SessionInterface->AddOnStartSessionCompleteDelegate_Handle(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete));
	SessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(FOnUpdateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnUpdateSessionComplete));
	SessionInterface->AddOnEndSessionCompleteDelegate_Handle(FOnEndSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnEndSessionComplete));
	SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete));
	SessionInterface->AddOnDestroySessionRequestedDelegate_Handle(FOnDestroySessionRequestedDelegate::CreateUObject(this, &ThisClass::OnDestroySessionRequested));

	//	SessionInterface->AddOnMatchmakingCompleteDelegate_Handle(FOnMatchmakingCompleteDelegate::CreateUObject(this, &ThisClass::OnMatchmakingComplete));
	//	SessionInterface->AddOnCancelMatchmakingCompleteDelegate_Handle(FOnCancelMatchmakingCompleteDelegate::CreateUObject(this, &ThisClass::OnCancelMatchmakingComplete));

	SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete));
	// 	SessionInterface->AddOnCancelFindSessionsCompleteDelegate_Handle(FOnCancelFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnCancelFindSessionsComplete));
	// 	SessionInterface->AddOnPingSearchResultsCompleteDelegate_Handle(FOnPingSearchResultsCompleteDelegate::CreateUObject(this, &ThisClass::OnPingSearchResultsComplete));
	SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));

	//	TWO_PARAM(OnSessionParticipantJoined, FName, const FUniqueNetId&);
	//	THREE_PARAM(OnSessionParticipantLeft, FName, const FUniqueNetId&, EOnSessionParticipantLeftReason);
	//	ONE_PARAM(OnQosDataRequested, FName);
	//	TWO_PARAM(OnSessionCustomDataChanged, FName, const FOnlineSessionSettings&);
	//	TWO_PARAM(OnSessionSettingsUpdated, FName, const FOnlineSessionSettings&);
	//	THREE_PARAM(OnSessionParticipantSettingsUpdated, FName, const FUniqueNetId&, const FOnlineSessionSettings&);
	//	FOUR_PARAM(OnSessionInviteReceived, const FUniqueNetId& /*UserId*/, const FUniqueNetId& /*FromId*/, const FString& /*AppId*/, const FOnlineSessionSearchResult& /*InviteResult*/);
	//	THREE_PARAM(OnRegisterPlayersComplete, FName, const TArray< FUniqueNetIdRef >&, bool);
	//	THREE_PARAM(OnUnregisterPlayersComplete, FName, const TArray< FUniqueNetIdRef >&, bool);

	SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &ThisClass::HandleSessionUserInviteAccepted));
	SessionInterface->AddOnSessionFailureDelegate_Handle(FOnSessionFailureDelegate::CreateUObject(this, &ThisClass::HandleSessionFailure));
}

void UOnlineSessionSubsystem::CreateOnlineSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UOnlineSession_HostSessionRequest* Request)
{
	const FName SessionName(NAME_GameSession);
	const int32 MaxPlayers = Request->GetMaxPlayers();

	IOnlineSubsystem* const OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions);

	FUniqueNetIdPtr UserId;
	if (LocalPlayer)
	{
		UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
	}
	else if (bIsDedicatedServer)
	{
		UserId = OnlineSub->GetIdentityInterface()->GetUniquePlayerId(DEDICATED_SERVER_USER_INDEX);
	}

	//@TODO: 在某些平台上，您可以在尝试进行局域网会话时到达这里，这需要有效的用户id吗？
	if (ensure(UserId.IsValid()))
	{
		FOnlineSession_OnlineSessionSettings HostSettings(Request->OnlineMode == EOnlineSessionOnlineMode::LAN, Request->bUsePresence, MaxPlayers);
		HostSettings.bUseLobbiesIfAvailable = Request->bUseLobbies;
		HostSettings.bUseLobbiesVoiceChatIfAvailable = Request->bUseLobbiesVoiceChat;
		HostSettings.Set(SETTING_GAMEMODE, Request->ModeNameForAdvertisement, EOnlineDataAdvertisementType::ViaOnlineService);
		HostSettings.Set(SETTING_MAPNAME, Request->GetMapName(), EOnlineDataAdvertisementType::ViaOnlineService);
		//@TODO: HostSettings.Set(SETTING_MATCHING_HOPPER, FString("TeamDeathmatch"), EOnlineDataAdvertisementType::DontAdvertise);
		HostSettings.Set(SETTING_MATCHING_TIMEOUT, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);
		HostSettings.Set(SETTING_SESSION_TEMPLATE_NAME, FString(TEXT("GameSession")), EOnlineDataAdvertisementType::ViaOnlineService);
		HostSettings.Set(SETTING_ONLINESUBSYSTEM_VERSION, true, EOnlineDataAdvertisementType::ViaOnlineService);

		Sessions->CreateSession(*UserId, SessionName, HostSettings);
		NotifySessionInformationUpdated(EOnlineSessionInformationState::InGame, Request->ModeNameForAdvertisement, Request->GetMapName());
	}
	else
	{
		OnCreateSessionComplete(SessionName, false);
	}
}

void UOnlineSessionSubsystem::FindSessionsInternalOSSv1(ULocalPlayer* LocalPlayer)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions);

	SearchSettings->QuerySettings.Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"), EOnlineComparisonOp::Equals);

	if (!Sessions->FindSessions(*LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), StaticCastSharedRef<FOnlineOnlineSearchSettingsOSSv1>(SearchSettings.ToSharedRef())))
	{
		// Some session search failures will call this delegate inside the function, others will not
		OnFindSessionsComplete(false);
	}
}

void UOnlineSessionSubsystem::JoinSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UOnlineSession_SearchResult* Request)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions);

	// We need to manually set that we want this to be our presence session
	Request->Result.Session.SessionSettings.bUsesPresence = true;
	Request->Result.Session.SessionSettings.bUseLobbiesIfAvailable = bUseLobbiesDefault;

	Sessions->JoinSession(*LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, Request->Result);
}

TSharedRef<FOnlineOnlineSearchSettings> UOnlineSessionSubsystem::CreateQuickPlaySearchSettingsOSSv1(UOnlineSession_HostSessionRequest* Request, UOnlineSession_SearchSessionRequest* QuickPlayRequest)
{
	TSharedRef<FOnlineOnlineSearchSettingsOSSv1> QuickPlaySearch = MakeShared<FOnlineOnlineSearchSettingsOSSv1>(QuickPlayRequest);

	/** By default quick play does not want to include the map or game mode, games can fill this in as desired
	if (!HostRequest->ModeNameForAdvertisement.IsEmpty())
	{
		QuickPlaySearch->QuerySettings.Set(SETTING_GAMEMODE, HostRequest->ModeNameForAdvertisement, EOnlineComparisonOp::Equals);
	}

	if (!HostRequest->GetMapName().IsEmpty())
	{
		QuickPlaySearch->QuerySettings.Set(SETTING_MAPNAME, HostRequest->GetMapName(), EOnlineComparisonOp::Equals);
	} 
	*/

	// QuickPlaySearch->QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
	return QuickPlaySearch;
}

void UOnlineSessionSubsystem::CleanUpSessionsOSSv1()
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions);

	EOnlineSessionState::Type SessionState = Sessions->GetSessionState(NAME_GameSession);
	UE_LOG(LogOnlineSession, Log, TEXT("Session state is %s"), EOnlineSessionState::ToString(SessionState));

	if (EOnlineSessionState::InProgress == SessionState)
	{
		UE_LOG(LogOnlineSession, Log, TEXT("Ending session because of return to front end"));
		Sessions->EndSession(NAME_GameSession);
	}
	else if (EOnlineSessionState::Ending == SessionState)
	{
		UE_LOG(LogOnlineSession, Log, TEXT("Waiting for session to end on return to main menu"));
	}
	else if (EOnlineSessionState::Ended == SessionState || EOnlineSessionState::Pending == SessionState)
	{
		UE_LOG(LogOnlineSession, Log, TEXT("Destroying session on return to main menu"));
		Sessions->DestroySession(NAME_GameSession);
	}
	else if (EOnlineSessionState::Starting == SessionState || EOnlineSessionState::Creating == SessionState)
	{
		UE_LOG(LogOnlineSession, Log, TEXT("Waiting for session to start, and then we will end it to return to main menu"));
	}
}

void UOnlineSessionSubsystem::HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType)
{
	UE_LOG(LogOnlineSession, Warning, TEXT("UOnlineSessionSubsystem::HandleSessionFailure(NetId: %s, FailureType: %s)"), *NetId.ToDebugString(), LexToString(FailureType));

	//@TODO: Probably need to do a bit more...
}

void UOnlineSessionSubsystem::HandleSessionUserInviteAccepted(const bool bWasSuccessful, const int32 LocalUserIndex, FUniqueNetIdPtr AcceptingUserId, const FOnlineSessionSearchResult& SearchResult)
{
	FPlatformUserId PlatformUserId = IPlatformInputDeviceMapper::Get().GetPlatformUserForUserIndex(LocalUserIndex);

	UOnlineSession_SearchResult* RequestedSession = nullptr;
	FOnlineResultInformation ResultInfo;
	if (bWasSuccessful)
	{
		RequestedSession = NewObject<UOnlineSession_SearchResult>(this);
		RequestedSession->Result = SearchResult;
	}
	else
	{
		// No FOnlineError to initialize from
		ResultInfo.bWasSuccessful = false;
		ResultInfo.ErrorId = TEXT("failed"); // This is not robust but there is no extended information available
		ResultInfo.ErrorText = LOCTEXT("Error_SessionUserInviteAcceptedFailed", "Failed to handle the join request");
	}
	NotifyUserRequestedSession(PlatformUserId, RequestedSession, ResultInfo);
}

void UOnlineSessionSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnlineSession, Log, TEXT("OnStartSessionComplete(SessionName: %s, bWasSuccessful: %d)"), *SessionName.ToString(), bWasSuccessful);

	if (bWantToDestroyPendingSession)
	{
		CleanUpSessions();
	}
}

void UOnlineSessionSubsystem::OnRegisterLocalPlayerComplete_CreateSession(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result)
{
	FinishSessionCreation(Result == EOnJoinSessionCompleteResult::Success);
}

void UOnlineSessionSubsystem::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnlineSession, Log, TEXT("OnUpdateSessionComplete(SessionName: %s, bWasSuccessful: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
}

void UOnlineSessionSubsystem::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnlineSession, Log, TEXT("OnEndSessionComplete(SessionName: %s, bWasSuccessful: %s)"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	CleanUpSessions();
}

void UOnlineSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnlineSession, Log, TEXT("OnDestroySessionComplete(SessionName: %s, bWasSuccessful: %s)"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	bWantToDestroyPendingSession = false;
}

void UOnlineSessionSubsystem::OnDestroySessionRequested(int32 LocalUserNum, FName SessionName)
{
	FPlatformUserId PlatformUserId = IPlatformInputDeviceMapper::Get().GetPlatformUserForUserIndex(LocalUserNum);

	NotifyDestroySessionRequested(PlatformUserId, SessionName);
}

void UOnlineSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogOnlineSession, Log, TEXT("OnFindSessionsComplete(bWasSuccessful: %s)"), bWasSuccessful ? TEXT("true") : TEXT("false"));

	if (!SearchSettings.IsValid())
	{
		// This could get called twice for failed session searches, or for a search requested by a different system
		return;
	}

	FOnlineOnlineSearchSettingsOSSv1& SearchSettingsV1 = *StaticCastSharedPtr<FOnlineOnlineSearchSettingsOSSv1>(SearchSettings);
	if (SearchSettingsV1.SearchState == EOnlineAsyncTaskState::InProgress)
	{
		UE_LOG(LogOnlineSession, Error, TEXT("OnFindSessionsComplete called when search is still in progress!"));
		return;
	}

	if (!ensure(SearchSettingsV1.SearchRequest))
	{
		UE_LOG(LogOnlineSession, Error, TEXT("OnFindSessionsComplete called with invalid search request object!"));
		return;
	}

	if (bWasSuccessful)
	{
		SearchSettingsV1.SearchRequest->Results.Reset(SearchSettingsV1.SearchResults.Num());

		for (const FOnlineSessionSearchResult& Result : SearchSettingsV1.SearchResults)
		{
			check(Result.IsValid());

			UOnlineSession_SearchResult* Entry = NewObject<UOnlineSession_SearchResult>(SearchSettingsV1.SearchRequest);
			Entry->Result = Result;
			SearchSettingsV1.SearchRequest->Results.Add(Entry);

			FString SessionId = TEXT("Unknown");
			if (Result.Session.SessionInfo.IsValid())
			{
				SessionId = Result.Session.SessionInfo->GetSessionId().ToString();
			}

			FString OwningUserId = TEXT("Unknown");
			if (Result.Session.OwningUserId.IsValid())
			{
				OwningUserId = Result.Session.OwningUserId->ToString();
			}

			UE_LOG(LogOnlineSession, Log, TEXT("\tFound session (SessionId: %s, UserId: %s, UserName: %s, NumOpenPrivConns: %d, NumOpenPubConns: %d, Ping: %d ms"),
			       *SessionId,
			       *OwningUserId,
			       *Result.Session.OwningUserName,
			       Result.Session.NumOpenPrivateConnections,
			       Result.Session.NumOpenPublicConnections,
			       Result.PingInMs
			);
		}
	}
	else
	{
		SearchSettingsV1.SearchRequest->Results.Empty();
	}

	if (0)
	{
		// Fake Sessions OSSV1
		for (int i = 0; i < 10; i++)
		{
			UOnlineSession_SearchResult* Entry = NewObject<UOnlineSession_SearchResult>(SearchSettings->SearchRequest);
			FOnlineSessionSearchResult FakeResult;
			FakeResult.Session.OwningUserName = TEXT("Fake User");
			FakeResult.Session.SessionSettings.NumPublicConnections = 10;
			FakeResult.Session.SessionSettings.bShouldAdvertise = true;
			FakeResult.Session.SessionSettings.bAllowJoinInProgress = true;
			FakeResult.PingInMs = 99;
			Entry->Result = FakeResult;
			SearchSettingsV1.SearchRequest->Results.Add(Entry);
		}
	}

	SearchSettingsV1.SearchRequest->NotifySearchFinished(bWasSuccessful, bWasSuccessful ? FText() : LOCTEXT("Error_FindSessionV1Failed", "Find session failed"));
	SearchSettings.Reset();
}

void UOnlineSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	// Add any splitscreen players if they exist
	//@TODO:
	// 	if (Result == EOnJoinSessionCompleteResult::Success && LocalPlayers.Num() > 1)
	// 	{
	// 		IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
	// 		if (Sessions.IsValid() && LocalPlayers[1]->GetPreferredUniqueNetId().IsValid())
	// 		{
	// 			Sessions->RegisterLocalPlayer(*LocalPlayers[1]->GetPreferredUniqueNetId(), NAME_GameSession,
	// 				FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &UShooterGameInstance::OnRegisterJoiningLocalPlayerComplete));
	// 		}
	// 	}
	// 	else
	{
		FinishJoinSession(Result);
	}
}

void UOnlineSessionSubsystem::OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result)
{
	FinishJoinSession(Result);
}

void UOnlineSessionSubsystem::FinishJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (bUseBeacons)
		{
			// InternalTravelToSession and the notification will be called by the beacon after a successful reservation. The beacon will be destroyed during travel.
			ConnectToHostReservationBeacon();
		}
		else
		{
			//@TODO Synchronize timing of this with create callbacks, modify both places and the comments if plan changes
			FOnlineResultInformation JoinSessionResult;
			JoinSessionResult.bWasSuccessful = true;
			NotifyJoinSessionComplete(JoinSessionResult);

			InternalTravelToSession(NAME_GameSession);
		}
	}
	else
	{
		FText ReturnReason;
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:
			ReturnReason = NSLOCTEXT("NetworkErrors", "SessionIsFull", "Game is full.");
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			ReturnReason = NSLOCTEXT("NetworkErrors", "SessionDoesNotExist", "Game no longer exists.");
			break;
		default:
			ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join failed.");
			break;
		}

		//@TODO: Error handling
		UE_LOG(LogOnlineSession, Error, TEXT("FinishJoinSession(Failed with Result: %s)"), *ReturnReason.ToString());

		// No FOnlineError to initialize from
		FOnlineResultInformation JoinSessionResult;
		JoinSessionResult.bWasSuccessful = false;
		JoinSessionResult.ErrorId = LexToString(Result); // This is not robust but there is no extended information available
		JoinSessionResult.ErrorText = ReturnReason;
		NotifyJoinSessionComplete(JoinSessionResult);
		NotifySessionInformationUpdated(EOnlineSessionInformationState::OutOfGame);

		// If the session join failed, we'll clean up the session
		CleanUpSessions();
	}
}
#endif

#if !ONLINEUSER_OSSV1
void UOnlineSessionSubsystem::BindOnlineDelegatesOSSv2()
{
	// TODO: 当OSSv2代理可用时，将其绑定
	//请注意，上述大多数OSSv1委托都是作为OSSv2中的完成委托实现的，不需要订阅
	TSharedPtr<IOnlineServices> OnlineServices = GetServices(GetWorld());
	check(OnlineServices);
	ILobbiesPtr Lobbies = OnlineServices->GetLobbiesInterface();
	if (ensure(Lobbies))
		LobbyJoinRequestedHandle = Lobbies->OnUILobbyJoinRequested().Add(this, &UOnlineSessionSubsystem::OnLobbyJoinRequested);


	ISessionsPtr Sessions = OnlineServices->GetSessionsInterface();
	if (ensure(Sessions))
		SessionJoinRequestedHandle = Sessions->OnUISessionJoinRequested().Add(this, &UOnlineSessionSubsystem::OnSessionJoinRequested);
}

void UOnlineSessionSubsystem::CreateOnlineSessionInternalOSSv2(ULocalPlayer* LocalPlayer, UOnlineSession_HostSessionRequest* Request)
{
	const FName SessionName(NAME_GameSession);
	const int32 MaxPlayers = Request->GetMaxPlayers();
	IOnlineServicesPtr OnlineServices = GetServices(GetWorld());

	check(OnlineServices);

	FString ModeName = Request->ModeNameForAdvertisement;
	FString MapName = Request->GetMapName();

	if (!Request->bUseLobbies)
	{
		ISessionsPtr Sessions = OnlineServices->GetSessionsInterface();
		check(Sessions);
		FCreateSession::Params CreateParams;

		if (LocalPlayer)
		{
			CreateParams.LocalAccountId = LocalPlayer->GetPreferredUniqueNetId().GetV2();
		}
		else if (bIsDedicatedServer)
		{
			// TODO what should this do for v2?
		}

		CreateParams.SessionName = SessionName;
		CreateParams.bPresenceEnabled = Request->bUsePresence;
		CreateParams.SessionSettings.SchemaName = FSchemaId(TEXT("GameLobby")); // TODO: make a parameter
		CreateParams.SessionSettings.NumMaxConnections = MaxPlayers;
		CreateParams.SessionSettings.JoinPolicy = ESessionJoinPolicy::Public; // TODO: Check parameters

		CreateParams.SessionSettings.CustomSettings.Emplace(SETTING_GAMEMODE, Request->ModeNameForAdvertisement);
		CreateParams.SessionSettings.CustomSettings.Emplace(SETTING_MAPNAME, Request->GetMapName());
		//@TODO: CreateParams.CustomSettings.Emplace(SETTING_MATCHING_HOPPER, FString("TeamDeathmatch"));
		CreateParams.SessionSettings.CustomSettings.Emplace(SETTING_MATCHING_TIMEOUT, 120.0f);
		CreateParams.SessionSettings.CustomSettings.Emplace(SETTING_SESSION_TEMPLATE_NAME, FString(TEXT("GameSession")));
		CreateParams.SessionSettings.CustomSettings.Emplace(SETTING_ONLINESUBSYSTEM_VERSION, true);

		Sessions->CreateSession(MoveTemp(CreateParams)).OnComplete(this, [this, SessionName, ModeName, MapName](const TOnlineResult<FCreateSession>& CreateResult)
		{
			OnCreateSessionComplete(SessionName, CreateResult.IsOk());
			NotifySessionInformationUpdated(EOnlineSessionInformationState::InGame, ModeName, MapName);
		});
	}
	else
	{
		ILobbiesPtr Lobbies = OnlineServices->GetLobbiesInterface();
		check(Lobbies);
		FCreateLobby::Params CreateParams;

		if (LocalPlayer)
		{
			CreateParams.LocalAccountId = LocalPlayer->GetPreferredUniqueNetId().GetV2();
		}
		else if (bIsDedicatedServer)
		{
			// TODO what should this do for v2?
		}

		CreateParams.LocalName = SessionName;
		CreateParams.SchemaId = FSchemaId(TEXT("GameLobby")); // TODO: make a parameter
		CreateParams.bPresenceEnabled = Request->bUsePresence;
		CreateParams.MaxMembers = MaxPlayers;
		CreateParams.JoinPolicy = ELobbyJoinPolicy::PublicAdvertised; // TODO: Check parameters

		CreateParams.Attributes.Emplace(SETTING_GAMEMODE, Request->ModeNameForAdvertisement);
		CreateParams.Attributes.Emplace(SETTING_MAPNAME, Request->GetMapName());
		//@TODO: CreateParams.Attributes.Emplace(SETTING_MATCHING_HOPPER, FString("TeamDeathmatch"));
		CreateParams.Attributes.Emplace(SETTING_MATCHING_TIMEOUT, 120.0f);
		CreateParams.Attributes.Emplace(SETTING_SESSION_TEMPLATE_NAME, FString(TEXT("GameSession")));
		CreateParams.Attributes.Emplace(SETTING_ONLINESUBSYSTEM_VERSION, true);

		CreateParams.UserAttributes.Emplace(SETTING_GAMEMODE, FString(TEXT("GameSession")));

		// TODO: Add splitscreen players

		Lobbies->CreateLobby(MoveTemp(CreateParams)).OnComplete(this, [this, SessionName, ModeName, MapName](const TOnlineResult<FCreateLobby>& CreateResult)
		{
			OnCreateSessionComplete(SessionName, CreateResult.IsOk());
			NotifySessionInformationUpdated(EOnlineSessionInformationState::InGame, ModeName, MapName);
		});
	}
}

void UOnlineSessionSubsystem::FindSessionsInternalOSSv2(ULocalPlayer* LocalPlayer)
{
	IOnlineServicesPtr OnlineServices = GetServices(GetWorld());
	check(OnlineServices);
	ILobbiesPtr Lobbies = OnlineServices->GetLobbiesInterface();
	check(Lobbies);

	FFindLobbies::Params FindLobbyParams = StaticCastSharedPtr<FOnlineOnlineSearchSettingsOSSv2>(SearchSettings)->FindLobbyParams;
	FindLobbyParams.LocalAccountId = LocalPlayer->GetPreferredUniqueNetId().GetV2();

	Lobbies->FindLobbies(MoveTemp(FindLobbyParams)).OnComplete(this, [this, LocalSearchSettings = SearchSettings](const TOnlineResult<FFindLobbies>& FindResult)
	{
		if (LocalSearchSettings != SearchSettings)
		{
			// This was an abandoned search, ignore
			return;
		}
		const bool bWasSuccessful = FindResult.IsOk();
		UE_LOG(LogOnlineSession, Log, TEXT("FindLobbies(bWasSuccessful: %s)"), *LexToString(bWasSuccessful));
		check(SearchSettings.IsValid());
		if (bWasSuccessful)
		{
			const FFindLobbies::Result& FindResults = FindResult.GetOkValue();
			SearchSettings->SearchRequest->Results.Reset(FindResults.Lobbies.Num());

			for (const TSharedRef<const FLobby>& Lobby : FindResults.Lobbies)
			{
				if (!Lobby->OwnerAccountId.IsValid())
				{
					UE_LOG(LogOnlineSession, Verbose, TEXT("\tIgnoring Lobby with no owner (LobbyId: %s)"),
					       *ToLogString(Lobby->LobbyId));
				}
				else if (Lobby->Members.Num() == 0)
				{
					UE_LOG(LogOnlineSession, Verbose, TEXT("\tIgnoring Lobby with no members (UserId: %s)"),
					       *ToLogString(Lobby->OwnerAccountId));
				}
				else
				{
					UOnlineSession_SearchResult* Entry = NewObject<UOnlineSession_SearchResult>(SearchSettings->SearchRequest);
					Entry->Lobby = Lobby;
					SearchSettings->SearchRequest->Results.Add(Entry);

					UE_LOG(LogOnlineSession, Log, TEXT("\tFound lobby (UserId: %s, NumOpenConns: %d)"),
					       *ToLogString(Lobby->OwnerAccountId), Lobby->MaxMembers - Lobby->Members.Num());
				}
			}
		}
		else
		{
			SearchSettings->SearchRequest->Results.Empty();
		}

		const FText ResultText = bWasSuccessful ? FText() : FindResult.GetErrorValue().GetText();

		SearchSettings->SearchRequest->NotifySearchFinished(bWasSuccessful, ResultText);
		SearchSettings.Reset();
	});
}

void UOnlineSessionSubsystem::JoinSessionInternalOSSv2(ULocalPlayer* LocalPlayer, UOnlineSession_SearchResult* Request)
{
	const FName SessionName(NAME_GameSession);
	IOnlineServicesPtr OnlineServices = GetServices(GetWorld());
	check(OnlineServices);

	// If the request doesnt have a lobby assume it's a session
	if (!Request->Lobby.IsValid())
	{
		ISessionsPtr Sessions = OnlineServices->GetSessionsInterface();
		check(Sessions);
		FJoinSession::Params CreateParams;

		if (LocalPlayer)
		{
			CreateParams.LocalAccountId = LocalPlayer->GetPreferredUniqueNetId().GetV2();
		}
		CreateParams.SessionName = SessionName;
		CreateParams.SessionId = Request->SessionID;
		UE::Online::FOnlineSessionId SessionID = Request->SessionID;

		Sessions->JoinSession(MoveTemp(CreateParams)).OnComplete(this, [this, SessionName](const TOnlineResult<FJoinSession>& JoinResult)
		{
			if (JoinResult.IsOk())
			{
				InternalTravelToSession(SessionName);
			}
			else
			{
				//@TODO: Error handling
				UE_LOG(LogOnlineSession, Error, TEXT("JoinLobby Failed with Result: %s"), *ToLogString(JoinResult.GetErrorValue()));
			}
		});
	}
	else
	{
		ILobbiesPtr Lobbies = OnlineServices->GetLobbiesInterface();
		check(Lobbies);

		FJoinLobby::Params JoinParams;
		if (LocalPlayer)
		{
			JoinParams.LocalAccountId = LocalPlayer->GetPreferredUniqueNetId().GetV2();
		}
		JoinParams.LocalName = SessionName;
		JoinParams.LobbyId = Request->Lobby->LobbyId;
		JoinParams.bPresenceEnabled = true;

		// Add any splitscreen players if they exist //@TODO: See UOnlineSessionSubsystem::OnJoinSessionComplete

		Lobbies->JoinLobby(MoveTemp(JoinParams)).OnComplete(this, [this, SessionName](const TOnlineResult<FJoinLobby>& JoinResult)
		{
			if (JoinResult.IsOk())
			{
				InternalTravelToSession(SessionName);
			}
			else
			{
				//@TODO: Error handling
				UE_LOG(LogOnlineSession, Error, TEXT("JoinLobby Failed with Result: %s"), *ToLogString(JoinResult.GetErrorValue()));
			}
		});
	}
}

TSharedRef<FOnlineOnlineSearchSettings> UOnlineSessionSubsystem::CreateQuickPlaySearchSettingsOSSv2(UOnlineSession_HostSessionRequest* HostRequest, UOnlineSession_SearchSessionRequest* SearchRequest)
{
	TSharedRef<FOnlineOnlineSearchSettingsOSSv2> QuickPlaySearch = MakeShared<FOnlineOnlineSearchSettingsOSSv2>(SearchRequest);

	/** By default quick play does not want to include the map or game mode, games can fill this in as desired
	if (!HostRequest->ModeNameForAdvertisement.IsEmpty())
	{
		QuickPlaySearch->FindLobbyParams.Filters.Emplace(FFindLobbySearchFilter{SETTING_GAMEMODE, ESchemaAttributeComparisonOp::Equals, HostRequest->ModeNameForAdvertisement});
	}
	if (!HostRequest->GetMapName().IsEmpty())
	{
		QuickPlaySearch->FindLobbyParams.Filters.Emplace(FFindLobbySearchFilter{SETTING_MAPNAME, ESchemaAttributeComparisonOp::Equals, HostRequest->GetMapName()});
	}
	*/

	return QuickPlaySearch;
}

void UOnlineSessionSubsystem::CleanUpSessionsOSSv2()
{
	IOnlineServicesPtr OnlineServices = GetServices(GetWorld());
	check(OnlineServices);
	ILobbiesPtr Lobbies = OnlineServices->GetLobbiesInterface();
	ISessionsPtr Sessions = OnlineServices->GetSessionsInterface();

	FAccountId LocalPlayerId = GetAccountId(GetGameInstance()->GetFirstLocalPlayerController());

	if (!LocalPlayerId.IsValid())
	{
		return;
	}

	if (bUseLobbiesDefault)
	{
		FLobbyId LobbyId = GetLobbyId(NAME_GameSession);

		if (!LobbyId.IsValid())
		{
			return;
		}
		// TODO:  Include all local players leave the lobby/session
		if (ensure(Lobbies))
		{
			Lobbies->LeaveLobby({LocalPlayerId, LobbyId});
		}
	}
	if (ensure(Sessions))
	{
		Sessions->LeaveSession({LocalPlayerId, NAME_GameSession, false});
	}
}

void UOnlineSessionSubsystem::OnLobbyJoinRequested(const UE::Online::FUILobbyJoinRequested& EventParams)
{
	TSharedPtr<IOnlineServices> OnlineServices = GetServices(GetWorld());
	check(OnlineServices);
	IAuthPtr Auth = OnlineServices->GetAuthInterface();
	check(Auth);
	TOnlineResult<FAuthGetLocalOnlineUserByOnlineAccountId> Account = Auth->GetLocalOnlineUserByOnlineAccountId({EventParams.LocalAccountId});
	if (Account.IsOk())
	{
		FPlatformUserId PlatformUserId = Account.GetOkValue().AccountInfo->PlatformUserId;
		UOnlineSession_SearchResult* RequestedSession = nullptr;
		FOnlineResultInformation ResultInfo;
		if (EventParams.Result.IsOk())
		{
			RequestedSession = NewObject<UOnlineSession_SearchResult>(this);
			RequestedSession->Lobby = EventParams.Result.GetOkValue();
		}
		else
		{
			ResultInfo.FromOnlineError(EventParams.Result.GetErrorValue());
		}
		NotifyUserRequestedSession(PlatformUserId, RequestedSession, ResultInfo);
	}
	else
	{
		UE_LOG(LogOnlineSession, Error, TEXT("OnJoinLobbyRequested: Failed to get account by local user id %s"), *UE::Online::ToLogString(EventParams.LocalAccountId));
	}
}

void UOnlineSessionSubsystem::OnSessionJoinRequested(const UE::Online::FUISessionJoinRequested& EventParams)
{
	TSharedPtr<IOnlineServices> OnlineServices = GetServices(GetWorld());
	check(OnlineServices);
	IAuthPtr Auth = OnlineServices->GetAuthInterface();
	check(Auth);
	TOnlineResult<FAuthGetLocalOnlineUserByOnlineAccountId> Account = Auth->GetLocalOnlineUserByOnlineAccountId({EventParams.LocalAccountId});
	if (Account.IsOk())
	{
		FPlatformUserId PlatformUserId = Account.GetOkValue().AccountInfo->PlatformUserId;
		UOnlineSession_SearchResult* RequestedSession = nullptr;
		FOnlineResultInformation ResultInfo;
		if (EventParams.Result.IsOk())
		{
			RequestedSession = NewObject<UOnlineSession_SearchResult>(this);
			RequestedSession->SessionID = EventParams.Result.GetOkValue();
		}
		else
		{
			ResultInfo.FromOnlineError(EventParams.Result.GetErrorValue());
		}
		NotifyUserRequestedSession(PlatformUserId, RequestedSession, ResultInfo);
	}
	else
	{
		UE_LOG(LogOnlineSession, Error, TEXT("OnJoinSessionRequested: Failed to get account by local user id %s"), *UE::Online::ToLogString(EventParams.LocalAccountId));
	}
}

UE::Online::FAccountId UOnlineSessionSubsystem::GetAccountId(APlayerController* PlayerController) const
{
	if (const ULocalPlayer* const LocalPlayer = PlayerController->GetLocalPlayer())
	{
		FUniqueNetIdRepl LocalPlayerIdRepl = LocalPlayer->GetPreferredUniqueNetId();
		if (LocalPlayerIdRepl.IsValid())
		{
			return LocalPlayerIdRepl.GetV2();
		}
	}
	return FAccountId();
}

UE::Online::FLobbyId UOnlineSessionSubsystem::GetLobbyId(const FName SessionName) const
{
	FAccountId LocalUserId = GetAccountId(GetGameInstance()->GetFirstLocalPlayerController());
	if (LocalUserId.IsValid())
	{
		IOnlineServicesPtr OnlineServices = GetServices(GetWorld());
		check(OnlineServices);
		ILobbiesPtr Lobbies = OnlineServices->GetLobbiesInterface();
		check(Lobbies);
		TOnlineResult<FGetJoinedLobbies> JoinedLobbies = Lobbies->GetJoinedLobbies({LocalUserId});
		if (JoinedLobbies.IsOk())
		{
			for (const TSharedRef<const FLobby>& Lobby : JoinedLobbies.GetOkValue().Lobbies)
			{
				if (Lobby->LocalName == SessionName)
				{
					return Lobby->LobbyId;
				}
			}
		}
	}
	return FLobbyId();
}

#endif

#undef LOCTEXT_NAMESPACE
