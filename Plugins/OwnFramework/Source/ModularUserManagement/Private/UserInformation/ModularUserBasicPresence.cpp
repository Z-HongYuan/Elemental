// Copyright © 2026 鸿源z. All Rights Reserved.


#include "UserInformation/ModularUserBasicPresence.h"

#include "Sessions/ModularUserSessionManager.h"


#if MODULARUSER_OSSV1
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlinePresenceInterface.h"
#endif

#if !MODULARUSER_OSSV1
#include "Online/OnlineServicesEngineUtils.h"
#include "Online/Presence.h"
#endif

DECLARE_LOG_CATEGORY_EXTERN(LogUserBasicPresence, Log, All); //注册Log分类
DEFINE_LOG_CATEGORY(LogUserBasicPresence); //注册Log分类

void UModularUserBasicPresence::Initialize(FSubsystemCollectionBase& Collection)
{
	UModularUserSessionManager* ModularSession = Collection.InitializeDependency<UModularUserSessionManager>();
	if (ensure(ModularSession))
		ModularSession->OnSessionInformationChangedEvent.AddUObject(this, &UModularUserBasicPresence::OnNotifySessionInformationChanged);
}

void UModularUserBasicPresence::Deinitialize()
{
	Super::Deinitialize();
}

void UModularUserBasicPresence::OnNotifySessionInformationChanged(EModularSessionInformationState SessionStatus, const FString& GameMode, const FString& MapName)
{
	if (bEnableSessionsBasedPresence && !GetGameInstance()->IsDedicatedServerInstance())
	{
		// 修剪地图名称，因为它是一个URL
		FString MapNameTruncated = MapName;
		if (!MapNameTruncated.IsEmpty())
		{
			int LastIndexOfSlash = 0;
			MapNameTruncated.FindLastChar('/', LastIndexOfSlash);
			MapNameTruncated = MapNameTruncated.RightChop(LastIndexOfSlash + 1);
		}

#if MODULARUSER_OSSV1
		IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
		if (OnlineSub)
		{
			IOnlinePresencePtr Presence = OnlineSub->GetPresenceInterface();
			if (Presence)
			{
				FOnlineUserPresenceStatus UpdatedPresence;
				UpdatedPresence.State = EOnlinePresenceState::Online; // 只有当用户具有有效的UniqueNetId时，我们才会发送状态更新，因此我们可以假设他们处于联机状态
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

#endif

#if !MODULARUSER_OSSV1

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

FString UModularUserBasicPresence::SessionStateToBackendKey(EModularSessionInformationState SessionStatus)
{
	switch (SessionStatus)
	{
	case EModularSessionInformationState::OutOfGame:
		return PresenceStatusMainMenu;
		break;
	case EModularSessionInformationState::Matchmaking:
		return PresenceStatusMatchmaking;
		break;
	case EModularSessionInformationState::InGame:
		return PresenceStatusInGame;
		break;
	default:
		UE_LOG(LogUserBasicPresence, Error, TEXT("UModularUserBasicPresence::SessionStateToBackendKey: Found unknown enum value %d"), (uint8)SessionStatus);
		return TEXT("Unknown");
		break;
	}
}
