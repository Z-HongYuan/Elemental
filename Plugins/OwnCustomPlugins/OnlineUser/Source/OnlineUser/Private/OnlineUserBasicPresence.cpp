// Copyright © 2026 鸿源z. All Rights Reserved.


#include "OnlineUserBasicPresence.h"

#include "OnlineSessionSubsystem.h"

#if ONLINEUSER_OSSV1
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlinePresenceInterface.h"
#endif

#if !ONLINEUSER_OSSV1
#include "Online/OnlineServicesEngineUtils.h"
#include "Online/Presence.h"
#endif

//定义日志分类
DECLARE_LOG_CATEGORY_EXTERN(LogUserBasicPresence, Log, All);

DEFINE_LOG_CATEGORY(LogUserBasicPresence);

UOnlineUserBasicPresence::UOnlineUserBasicPresence()
{
}

void UOnlineUserBasicPresence::Initialize(FSubsystemCollectionBase& Collection)
{
	/*向子系统集合声明：我需要 UOnlineSessionSubsystem 作为依赖项。如果它还没初始化，先初始化它；如果已经初始化返回它的指针。*/
	UOnlineSessionSubsystem* OnlineSession = Collection.InitializeDependency<UOnlineSessionSubsystem>();
	if (ensure(OnlineSession))
		OnlineSession->OnSessionInformationChangedEvent.AddUObject(this, &ThisClass::OnNotifySessionInformationChanged);
}

void UOnlineUserBasicPresence::Deinitialize()
{
	Super::Deinitialize();
}

void UOnlineUserBasicPresence::OnNotifySessionInformationChanged(EOnlineSessionInformationState SessionStatus, const FString& GameMode, const FString& MapName)
{
	if (bEnableSessionsBasedPresence && !GetGameInstance()->IsDedicatedServerInstance())
	{
		// trim the map name since its a URL
		FString MapNameTruncated = MapName;
		if (!MapNameTruncated.IsEmpty())
		{
			int LastIndexOfSlash = 0;
			MapNameTruncated.FindLastChar('/', LastIndexOfSlash);
			MapNameTruncated = MapNameTruncated.RightChop(LastIndexOfSlash + 1);
		}

#if ONLINEUSER_OSSV1
		if (IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld()))
		{
			IOnlinePresencePtr Presence = OnlineSub->GetPresenceInterface();
			if (Presence)
			{
				FOnlineUserPresenceStatus UpdatedPresence;
				UpdatedPresence.State = EOnlinePresenceState::Online; // We'll only send the presence update if the user has a valid UniqueNetId, so we can assume they are Online
				UpdatedPresence.StatusStr = *SessionStateToBackendKey(SessionStatus);
				UpdatedPresence.Properties.Emplace(PresenceKeyGameMode, GameMode);
				UpdatedPresence.Properties.Emplace(PresenceKeyMapName, MapNameTruncated);

				for (const ULocalPlayer* LocalPlayer : GetGameInstance()->GetLocalPlayers())
				{
					if (LocalPlayer && LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId() != nullptr)
					{
						Presence->SetPresence(*LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), UpdatedPresence);
					}
				}
			}
		}

#else

		UE::Online::IOnlineServicesPtr OnlineServices = UE::Online::GetServices(GetWorld());
		check(OnlineServices);
		UE::Online::IPresencePtr Presence = OnlineServices->GetPresenceInterface();
		if (Presence)
		{
			for (const ULocalPlayer* LocalPlayer : GetGameInstance()->GetLocalPlayers())
			{
				if (LocalPlayer && LocalPlayer->GetPreferredUniqueNetId().IsV2())
				{
					UE::Online::FPartialUpdatePresence::Params UpdateParams;
					UpdateParams.LocalAccountId = LocalPlayer->GetPreferredUniqueNetId().GetV2();
					UpdateParams.Mutations.StatusString.Emplace(*SessionStateToBackendKey(SessionStatus));
					UpdateParams.Mutations.UpdatedProperties.AddVariant(PresenceKeyGameMode, GameMode);
					UpdateParams.Mutations.UpdatedProperties.AddVariant(PresenceKeyMapName, MapNameTruncated);

					Presence->PartialUpdatePresence(MoveTemp(UpdateParams));
				}
			}
		}
#endif
	}
}

FString UOnlineUserBasicPresence::SessionStateToBackendKey(EOnlineSessionInformationState SessionStatus)
{
	switch (SessionStatus)
	{
	case EOnlineSessionInformationState::OutOfGame:
		return PresenceStatusMainMenu;
		break;
	case EOnlineSessionInformationState::Matchmaking:
		return PresenceStatusMatchmaking;
		break;
	case EOnlineSessionInformationState::InGame:
		return PresenceStatusInGame;
		break;
	default:
		UE_LOG(LogUserBasicPresence, Error, TEXT("UOnlineUserBasicPresence::SessionStateToBackendKey: Found unknown enum value %d"), (uint8)SessionStatus);
		return TEXT("Unknown");
		break;
	}
}
