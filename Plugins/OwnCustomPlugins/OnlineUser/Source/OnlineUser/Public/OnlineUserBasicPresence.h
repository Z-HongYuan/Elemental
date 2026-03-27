// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineUserBasicPresence.generated.h"

#define UE_API ONLINEUSER_API

enum class EOnlineSessionInformationState : uint8;
/**
 * 该子系统插入会话子系统，并将其信息推送到状态界面。
 */
UCLASS(MinimalAPI, BlueprintType, Config = Engine)
class UOnlineUserBasicPresence : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UE_API UOnlineUserBasicPresence();

	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;

	/** False是一个通用的kills开关，用于阻止该类推送存在*/
	UPROPERTY(Config)
	bool bEnableSessionsBasedPresence = false;

	/** 将“游戏中”的状态映射到后端密钥*/
	UPROPERTY(Config)
	FString PresenceStatusInGame;

	/** 将状态“主菜单”映射到后端键*/
	UPROPERTY(Config)
	FString PresenceStatusMainMenu;

	/** 将状态“配对”映射到后端密钥*/
	UPROPERTY(Config)
	FString PresenceStatusMatchmaking;

	/** 将“游戏模式”的丰富显示条目映射到后端密钥*/
	UPROPERTY(Config)
	FString PresenceKeyGameMode;

	/** 将“Map Name”丰富的在线状态条目映射到后端密钥*/
	UPROPERTY(Config)
	FString PresenceKeyMapName;

	UE_API void OnNotifySessionInformationChanged(EOnlineSessionInformationState SessionStatus, const FString& GameMode, const FString& MapName);
	UE_API FString SessionStateToBackendKey(EOnlineSessionInformationState SessionStatus);
};

#undef UE_API
