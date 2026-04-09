// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "ModularSession_HostSessionRequest.generated.h"

#define UE_API MODULARUSERMANAGEMENT_API

/** 指定游戏会话应使用的联机功能和连接 */
UENUM(BlueprintType)
enum class EModularSessionOnlineMode : uint8
{
	Offline, //离线
	LAN, //局域网
	Online //在线
};

/**
 * 存储会话使用的参数的请求对象
 */
UCLASS(MinimalAPI, BlueprintType)
class UModularSession_HostSessionRequest : public UObject
{
	GENERATED_BODY()

public:
	/** 指示会话是完整联机会话还是其他类型 */
	UPROPERTY(BlueprintReadWrite, Category=Session)
	EModularSessionOnlineMode OnlineMode;

	/** 如果此请求应创建玩家托管的大厅（如果可用），则为True*/
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUseLobbies;

	/** 如果此请求应创建一个支持语音聊天的大厅，则为True */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUseLobbiesVoiceChat;

	/** 如果此请求应创建将显示在用户状态信息中的会话，则为True */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUsePresence;

	/** 匹配过程中用于指定这是哪种游戏模式的字符串*/
	UPROPERTY(BlueprintReadWrite, Category=Session)
	FString ModeNameForAdvertisement;

	/** 游戏开始时加载的地图，这需要是一个有效的主要资产顶级地图*/
	UPROPERTY(BlueprintReadWrite, Category=Session, meta=(AllowedTypes="World"))
	FPrimaryAssetId MapID;

	/**额外的参数作为URL选项传递给游戏 */
	UPROPERTY(BlueprintReadWrite, Category=Session)
	TMap<FString, FString> ExtraArgs;

	/** 每个游戏会话允许的最大玩家数*/
	UPROPERTY(BlueprintReadWrite, Category=Session)
	int32 MaxPlayerCount = 16;

public:
	/** 返回实际应该使用的最大玩家数，可以在子类中覆盖*/
	UE_API virtual int32 GetMaxPlayers() const;

	/** 返回将在游戏过程中使用的完整地图名称*/
	UE_API virtual FString GetMapName() const;

	/** 构造将传递给ServerTravel的完整URL */
	UE_API virtual FString ConstructTravelURL() const;

	/** 如果此请求有效，则返回true，否则返回false并记录错误 */
	UE_API virtual bool ValidateAndLogErrors(FText& OutError) const;
};

#undef UE_API
