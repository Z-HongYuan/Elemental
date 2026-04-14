// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularGameHub/Public/ModularGameInstance.h"

#include "CommonUISettings.h"
#include "GameplayTagContainer.h"
#include "ICommonUIModule.h"
#include "Logs/ModularGameHubLogs.h"
#include "System/ModularUIManager.h"

int32 UModularGameInstance::AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId)
{
	int32 ReturnVal = Super::AddLocalPlayer(NewPlayer, UserId);
	if (ReturnVal != INDEX_NONE)
	{
		if (!PrimaryPlayer.IsValid())
		{
			UE_LOG(LogModularGameHub, Log, TEXT("AddLocalPlayer: Set %s to Primary Player"), *NewPlayer->GetName());
			PrimaryPlayer = NewPlayer;
		}

		GetSubsystem<UModularUIManager>()->NotifyPlayerAdded(NewPlayer);
	}

	return ReturnVal;
}

bool UModularGameInstance::RemoveLocalPlayer(ULocalPlayer* ExistingPlayer)
{
	if (PrimaryPlayer == ExistingPlayer)
	{
		//TODO: 是否需要回退找其他玩家？
		PrimaryPlayer.Reset();
		UE_LOG(LogModularGameHub, Log, TEXT("RemoveLocalPlayer: Unsetting Primary Player from %s"), *ExistingPlayer->GetName());
	}
	GetSubsystem<UModularUIManager>()->NotifyPlayerDestroyed(ExistingPlayer);

	return Super::RemoveLocalPlayer(ExistingPlayer);
}

void UModularGameInstance::Init()
{
	Super::Init();

	// 子系统初始化后，将它们组合在一起
	FGameplayTagContainer PlatformTraits = ICommonUIModule::GetSettings().GetPlatformTraits();

	// UModularUserManager* UserSubsystem = GetSubsystem<UModularUserManager>();
	// if (ensure(UserSubsystem))
	// {
	// 	UserSubsystem->SetTraitTags(PlatformTraits);
	// 	UserSubsystem->OnHandleSystemMessage.AddDynamic(this, &UModularGameInstance::HandleSystemMessage);
	// 	UserSubsystem->OnUserPrivilegeChanged.AddDynamic(this, &UModularGameInstance::HandlePrivilegeChanged);
	// 	UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &UModularGameInstance::HandlerUserInitialized);
	// }
	//
	// UModularUserSessionManager* SessionSubsystem = GetSubsystem<UModularUserSessionManager>();
	// if (ensure(SessionSubsystem))
	// {
	// 	SessionSubsystem->OnUserRequestedSessionEvent.AddUObject(this, &UModularGameInstance::OnUserRequestedSession);
	// 	SessionSubsystem->OnDestroySessionRequestedEvent.AddUObject(this, &UModularGameInstance::OnDestroySessionRequested);
	// }
}

void UModularGameInstance::ReturnToMainMenu()
{
	// 默认情况下，回到主菜单时我们应该重置所有内容
	// ResetUserAndSessionState();

	Super::ReturnToMainMenu();
}
