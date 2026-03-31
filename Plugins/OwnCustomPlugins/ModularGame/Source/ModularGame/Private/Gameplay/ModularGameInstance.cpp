// Copyright © 2026 鸿源z. All Rights Reserved.


#include "Gameplay/ModularGameInstance.h"

#include "CommonUISettings.h"
#include "GameplayTagContainer.h"
#include "ICommonUIModule.h"
#include "ModularGame.h"
#include "OnlineSessionSubsystem.h"
#include "OnlineUserSubsystem.h"
#include "Gameplay/ModularLocalPlayer.h"
#include "Message/ModularGameDialogDescriptor.h"
#include "Message/ModularMessagingSubsystem.h"
#include "Widgets/ModularUIManagerSubsystem.h"

UModularGameInstance::UModularGameInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UModularGameInstance::HandleSystemMessage(FGameplayTag MessageType, FText Title, FText Message)
{
	ULocalPlayer* FirstPlayer = GetFirstGamePlayer();
	//将严重错误转发到第一个玩家的错误对话框
	if (FirstPlayer && MessageType.MatchesTag(OnlineUserTags::OnlineUser_SystemMessage_Error))
	{
		if (UModularMessagingSubsystem* Messaging = FirstPlayer->GetSubsystem<UModularMessagingSubsystem>())
		{
			Messaging->ShowError(UModularGameDialogDescriptor::CreateConfirmationOk(Title, Message));
		}
	}
}

void UModularGameInstance::HandlePrivilegeChanged(const UOnlineUserInfo* UserInfo, EOnlineUserPrivilege Privilege, EOnlineUserAvailability OldAvailability, EOnlineUserAvailability NewAvailability)
{
	// 默认情况下，如果第一个玩家的播放权限丢失，则会显示错误并断开连接。
	if (Privilege == EOnlineUserPrivilege::CanPlay && OldAvailability == EOnlineUserAvailability::NowAvailable && NewAvailability != EOnlineUserAvailability::NowAvailable)
	{
		UE_LOG(LogModularGame, Error, TEXT("HandlePrivilegeChanged: Player %d no longer has permission to play the game!"), UserInfo->LocalPlayerIndex);
		// TODO：游戏可以在子类中执行一些特定操作
		// 返回主菜单();
		// ReturnToMainMenu();
	}
}

void UModularGameInstance::HandlerUserInitialized(const UOnlineUserInfo* UserInfo, bool bSuccess, FText Error, EOnlineUserPrivilege RequestedPrivilege, EOnlineUserOnlineContext OnlineContext)
{
	// Subclasses can override this
}

void UModularGameInstance::ResetUserAndSessionState()
{
	UOnlineUserSubsystem* UserSubsystem = GetSubsystem<UOnlineUserSubsystem>();
	if (ensure(UserSubsystem))
		UserSubsystem->ResetUserState();


	UOnlineSessionSubsystem* SessionSubsystem = GetSubsystem<UOnlineSessionSubsystem>();
	if (ensure(SessionSubsystem))
		SessionSubsystem->CleanUpSessions();
}

void UModularGameInstance::OnUserRequestedSession(const FPlatformUserId& PlatformUserId, UOnlineSession_SearchResult* InRequestedSession, const FOnlineResultInformation& RequestedSessionResult)
{
	if (InRequestedSession)
	{
		SetRequestedSession(InRequestedSession);
	}
	else
	{
		HandleSystemMessage(OnlineUserTags::OnlineUser_SystemMessage_Error, NSLOCTEXT("CommonGame", "Warning_RequestedSessionFailed", "Requested Session Failed"), RequestedSessionResult.ErrorText);
	}
}

void UModularGameInstance::OnDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName)
{
	// 当请求销毁会话时，请确保您的项目处于正确的状态，以便销毁会话并退出会话。
	UE_LOG(LogModularGame, Verbose, TEXT("[%hs] PlatformUserId:%d, SessionName: %s)"), __FUNCTION__, PlatformUserId.GetInternalId(), *SessionName.ToString());
	ReturnToMainMenu();
}

void UModularGameInstance::SetRequestedSession(UOnlineSession_SearchResult* InRequestedSession)
{
	RequestedSession = InRequestedSession;
	if (RequestedSession)
	{
		if (CanJoinRequestedSession())
		{
			JoinRequestedSession();
		}
		else
		{
			ResetGameAndJoinRequestedSession();
		}
	}
}

bool UModularGameInstance::CanJoinRequestedSession() const
{
	// 默认行为始终允许加入请求的会话
	return true;
}

void UModularGameInstance::JoinRequestedSession()
{
	if (RequestedSession)
	{
		if (ULocalPlayer* const FirstPlayer = GetFirstGamePlayer())
		{
			UOnlineSessionSubsystem* SessionSubsystem = GetSubsystem<UOnlineSessionSubsystem>();
			if (ensure(SessionSubsystem))
			{
				// 请清除我们当前请求的会话，因为我们现在正在执行操作。
				UOnlineSession_SearchResult* LocalRequestedSession = RequestedSession;
				RequestedSession = nullptr;
				SessionSubsystem->JoinSession(FirstPlayer->PlayerController, LocalRequestedSession);
			}
		}
	}
}

void UModularGameInstance::ResetGameAndJoinRequestedSession()
{
	//默认行为是返回主菜单。游戏必须在就绪状态时调用 JoinRequestedSession 函数。
	ReturnToMainMenu();
}

int32 UModularGameInstance::AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId)
{
	int32 ReturnVal = Super::AddLocalPlayer(NewPlayer, UserId);
	if (ReturnVal != INDEX_NONE)
	{
		if (!PrimaryPlayer.IsValid())
		{
			UE_LOG(LogModularGame, Log, TEXT("AddLocalPlayer: Set %s to Primary Player"), *NewPlayer->GetName());
			PrimaryPlayer = NewPlayer;
		}

		GetSubsystem<UModularUIManagerSubsystem>()->NotifyPlayerAdded(Cast<UModularLocalPlayer>(NewPlayer));
	}

	return ReturnVal;
}

bool UModularGameInstance::RemoveLocalPlayer(ULocalPlayer* ExistingPlayer)
{
	if (PrimaryPlayer == ExistingPlayer)
	{
		//TODO: do we want to fall back to another player?
		PrimaryPlayer.Reset();
		UE_LOG(LogModularGame, Log, TEXT("RemoveLocalPlayer: Unsetting Primary Player from %s"), *ExistingPlayer->GetName());
	}
	GetSubsystem<UModularUIManagerSubsystem>()->NotifyPlayerDestroyed(Cast<UModularLocalPlayer>(ExistingPlayer));

	return Super::RemoveLocalPlayer(ExistingPlayer);
}

void UModularGameInstance::Init()
{
	Super::Init();

	// After subsystems are initialized, hook them together
	FGameplayTagContainer PlatformTraits = ICommonUIModule::GetSettings().GetPlatformTraits();

	UOnlineUserSubsystem* UserSubsystem = GetSubsystem<UOnlineUserSubsystem>();
	if (ensure(UserSubsystem))
	{
		UserSubsystem->SetTraitTags(PlatformTraits);
		UserSubsystem->OnHandleSystemMessage.AddDynamic(this, &ThisClass::HandleSystemMessage);
		UserSubsystem->OnUserPrivilegeChanged.AddDynamic(this, &ThisClass::HandlePrivilegeChanged);
		UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &ThisClass::HandlerUserInitialized);
	}

	UOnlineSessionSubsystem* SessionSubsystem = GetSubsystem<UOnlineSessionSubsystem>();
	if (ensure(SessionSubsystem))
	{
		SessionSubsystem->OnUserRequestedSessionEvent.AddUObject(this, &ThisClass::OnUserRequestedSession);
		SessionSubsystem->OnDestroySessionRequestedEvent.AddUObject(this, &ThisClass::OnDestroySessionRequested);
	}
}

void UModularGameInstance::ReturnToMainMenu()
{
	// 默认情况下，返回主菜单时应该重置所有内容。
	ResetUserAndSessionState();

	Super::ReturnToMainMenu();
}
