// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "ModularUserBasicPresence.generated.h"

#define UE_API MODULARUSERMANAGEMENT_API

enum class EModularSessionInformationState : uint8;
/**
 * 该子系统插入会话子系统，并将其信息推送到状态界面。
 */
UCLASS(MinimalAPI, BlueprintType, Config = Engine)
class UModularUserBasicPresence : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 执行此操作以初始化系统实例 */
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 实现此操作以取消系统实例的初始化 */
	UE_API virtual void Deinitialize() override;

	/** False是一个通用的kills开关，用于阻止该类推送存在 */
	UPROPERTY(Config)
	bool bEnableSessionsBasedPresence = false;

	/** 提供给在线服务的 在游戏中状态 */
	UPROPERTY(Config)
	FString PresenceStatusInGame;

	/** 提供给在线服务的 位于主菜单状态 */
	UPROPERTY(Config)
	FString PresenceStatusMainMenu;

	/** 提供给在线服务的 匹配模式 */
	UPROPERTY(Config)
	FString PresenceStatusMatchmaking;

	/** 提供给在线服务的 GameMode */
	UPROPERTY(Config)
	FString PresenceKeyGameMode;

	/** 提供给在线服务的 MapName */
	UPROPERTY(Config)
	FString PresenceKeyMapName;

	UE_API void OnNotifySessionInformationChanged(EModularSessionInformationState SessionStatus, const FString& GameMode, const FString& MapName);
	UE_API FString SessionStateToBackendKey(EModularSessionInformationState SessionStatus);
};

#undef UE_API
