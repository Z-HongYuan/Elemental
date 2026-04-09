// Copyright © 2026 鸿源z. All Rights Reserved.


#include "Sessions/ModularUserSessionManager.h"

#include "Online/OnlineSessionNames.h"


#if MODULARUSER_OSSV1
#include "Engine/World.h"
#include "OnlineSubsystemUtils.h"

FName SETTING_ONLINESUBSYSTEM_VERSION(TEXT("OSSv1"));
#endif
#if !MODULARUSER_OSSV1
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Online/OnlineServicesEngineUtils.h"

FName SETTING_ONLINESUBSYSTEM_VERSION(TEXT("OSSv2"));
using namespace UE::Online;
#endif

DECLARE_LOG_CATEGORY_EXTERN(LogModularSession, Log, All); //定义Log分类
DEFINE_LOG_CATEGORY(LogModularSession); //定义Log分类

#define LOCTEXT_NAMESPACE "ModularUser"

class FModularOnlineSearchSettingsBase : public FGCObject
{
public:
	FModularOnlineSearchSettingsBase(UModularSession_SearchSessionRequest* InSearchRequest)
	{
		SearchRequest = InSearchRequest;
	}

	virtual ~FModularOnlineSearchSettingsBase() override
	{
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObject(SearchRequest);
	}

	virtual FString GetReferencerName() const override
	{
		static const FString NameString = TEXT("FModularOnlineSearchSettings");
		return NameString;
	}

public:
	TObjectPtr<UModularSession_SearchSessionRequest> SearchRequest = nullptr;
};


#if MODULARUSER_OSSV1
//////////////////////////////////////////////////////////////////////
// FModularSession_OnlineSessionSettings

class FModularSession_OnlineSessionSettings : public FOnlineSessionSettings
{
public:
	FModularSession_OnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4)
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

	virtual ~FModularSession_OnlineSessionSettings()
	{
	}
};

//////////////////////////////////////////////////////////////////////
// FModularOnlineSearchSettingsOSSv1

class FModularOnlineSearchSettingsOSSv1 : public FOnlineSessionSearch, public FModularOnlineSearchSettingsBase
{
public:
	FModularOnlineSearchSettingsOSSv1(UModularSession_SearchSessionRequest* InSearchRequest)
		: FModularOnlineSearchSettingsBase(InSearchRequest)
	{
		bIsLanQuery = (InSearchRequest->OnlineMode == EModularSessionOnlineMode::LAN);
		MaxSearchResults = 10;
		PingBucketSize = 50;

		QuerySettings.Set(SETTING_ONLINESUBSYSTEM_VERSION, true, EOnlineComparisonOp::Equals);

		if (InSearchRequest->bUseLobbies)
		{
			QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
		}
	}

	virtual ~FModularOnlineSearchSettingsOSSv1()
	{
	}
};
#endif

#if !MODULARUSER_OSSV1

class FModularOnlineSearchSettingsOSSv2 : public FModularOnlineSearchSettingsBase
{
public:
	FModularOnlineSearchSettingsOSSv2(UModularSession_SearchSessionRequest* InSearchRequest)
		: FModularOnlineSearchSettingsBase(InSearchRequest)
	{
		FindLobbyParams.MaxResults = 10;

		FindLobbyParams.Filters.Emplace(FFindLobbySearchFilter{SETTING_ONLINESUBSYSTEM_VERSION, ESchemaAttributeComparisonOp::Equals, true});
	}

public:
	FFindLobbies::Params FindLobbyParams;
};

#endif


void UModularUserSessionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BindOnlineDelegates();
	GEngine->OnTravelFailure().AddUObject(this, &UModularUserSessionManager::TravelLocalSessionFailure);

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UModularUserSessionManager::HandlePostLoadMap);

	UGameInstance* GameInstance = GetGameInstance();
	bIsDedicatedServer = GameInstance->IsDedicatedServerInstance();
}

void UModularUserSessionManager::Deinitialize()
{
	Super::Deinitialize();
}

bool UModularUserSessionManager::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer);
}

UModularSession_HostSessionRequest* UModularUserSessionManager::CreateOnlineHostSessionRequest()
{
	return nullptr;
}

UModularSession_SearchSessionRequest* UModularUserSessionManager::CreateOnlineSearchSessionRequest()
{
	return nullptr;
}

void UModularUserSessionManager::HostSession(APlayerController* HostingPlayer, UModularSession_HostSessionRequest* Request)
{
}

void UModularUserSessionManager::QuickPlaySession(APlayerController* JoiningOrHostingPlayer, UModularSession_HostSessionRequest* Request)
{
}

void UModularUserSessionManager::JoinSession(APlayerController* JoiningPlayer, UModularSession_SearchResult* Request)
{
}

void UModularUserSessionManager::FindSessions(APlayerController* SearchingPlayer, UModularSession_SearchSessionRequest* Request)
{
}

void UModularUserSessionManager::CleanUpSessions()
{
}

TSharedRef<FModularOnlineSearchSettings> UModularUserSessionManager::CreateQuickPlaySearchSettings(UModularSession_HostSessionRequest* Request, UModularSession_SearchSessionRequest* QuickPlayRequest)
{
	return {};
}

void UModularUserSessionManager::HandleQuickPlaySearchFinished(bool bSucceeded, const FText& ErrorMessage, TWeakObjectPtr<APlayerController> JoiningOrHostingPlayer, TStrongObjectPtr<UModularSession_HostSessionRequest> HostRequest)
{
}

void UModularUserSessionManager::TravelLocalSessionFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ReasonString)
{
}

void UModularUserSessionManager::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UModularUserSessionManager::FinishSessionCreation(bool bWasSuccessful)
{
}

void UModularUserSessionManager::HandlePostLoadMap(UWorld* World)
{
}

void UModularUserSessionManager::BindOnlineDelegates()
{
#if MODULARUSER_OSSV1
	BindOnlineDelegatesOSSv1();
#endif
#if !MODULARUSER_OSSV1
	BindOnlineDelegatesOSSv2();
#endif
}

void UModularUserSessionManager::CreateOnlineSessionInternal(ULocalPlayer* LocalPlayer, UModularSession_HostSessionRequest* Request)
{
}

void UModularUserSessionManager::FindSessionsInternal(APlayerController* SearchingPlayer, const TSharedRef<FModularOnlineSearchSettings>& InSearchSettings)
{
}

void UModularUserSessionManager::JoinSessionInternal(ULocalPlayer* LocalPlayer, UModularSession_SearchResult* Request)
{
}

void UModularUserSessionManager::InternalTravelToSession(const FName SessionName)
{
}

void UModularUserSessionManager::NotifyUserRequestedSession(const FPlatformUserId& PlatformUserId, UModularSession_SearchResult* RequestedSession, const FOnlineResultInformation& RequestedSessionResult)
{
}

void UModularUserSessionManager::NotifyJoinSessionComplete(const FOnlineResultInformation& Result)
{
}

void UModularUserSessionManager::NotifyCreateSessionComplete(const FOnlineResultInformation& Result)
{
}

void UModularUserSessionManager::NotifySessionInformationUpdated(EModularSessionInformationState SessionStatusStr, const FString& GameMode, const FString& MapName)
{
}

void UModularUserSessionManager::NotifyDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName)
{
}

void UModularUserSessionManager::SetCreateSessionError(const FText& ErrorText)
{
}

void UModularUserSessionManager::BindOnlineDelegatesOSSv1()
{
}

void UModularUserSessionManager::CreateOnlineSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UModularSession_HostSessionRequest* Request)
{
}

void UModularUserSessionManager::FindSessionsInternalOSSv1(ULocalPlayer* LocalPlayer)
{
}

void UModularUserSessionManager::JoinSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UModularSession_SearchResult* Request)
{
}

TSharedRef<FModularOnlineSearchSettings> UModularUserSessionManager::CreateQuickPlaySearchSettingsOSSv1(UModularSession_HostSessionRequest* Request, UModularSession_SearchSessionRequest* QuickPlayRequest)
{
	return {};
}

void UModularUserSessionManager::CleanUpSessionsOSSv1()
{
}

void UModularUserSessionManager::HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType)
{
}

void UModularUserSessionManager::HandleSessionUserInviteAccepted(const bool bWasSuccessful, const int32 LocalUserIndex, FUniqueNetIdPtr AcceptingUserId, const FOnlineSessionSearchResult& SearchResult)
{
}

void UModularUserSessionManager::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UModularUserSessionManager::OnRegisterLocalPlayerComplete_CreateSession(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result)
{
}

void UModularUserSessionManager::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UModularUserSessionManager::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UModularUserSessionManager::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UModularUserSessionManager::OnDestroySessionRequested(int32 LocalUserNum, FName SessionName)
{
}

void UModularUserSessionManager::OnFindSessionsComplete(bool bWasSuccessful)
{
}

void UModularUserSessionManager::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
}

void UModularUserSessionManager::OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result)
{
}

void UModularUserSessionManager::FinishJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
}

void UModularUserSessionManager::CreateHostReservationBeacon()
{
}

void UModularUserSessionManager::ConnectToHostReservationBeacon()
{
}

void UModularUserSessionManager::DestroyHostReservationBeacon()
{
}

#undef LOCTEXT_NAMESPACE
