// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "OnlineSessionSettings.h"
#include "UObject/Object.h"
#include "ModularSession_SearchResult.generated.h"

#define UE_API MODULARUSERMANAGEMENT_API

/**
 * 从在线系统返回的结果对象，描述了可加入的游戏会话
 */
UCLASS(MinimalAPI, BlueprintType)
class UModularSession_SearchResult : public UObject
{
	GENERATED_BODY()

public:
	/**返回会话的内部描述，不便于人类阅读*/
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API FString GetDescription() const;

	/** 获取任意字符串设置，如果该设置不存在，bFoundValue将为false */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API void GetStringSetting(FName Key, FString& Value, bool& bFoundValue) const;

	/** 获取任意整数设置，如果该设置不存在，bFoundValue将为false */
	UFUNCTION(BlueprintPure, Category = Sessions)
	UE_API void GetIntSetting(FName Key, int32& Value, bool& bFoundValue) const;

	/** 可用的专用连接数 */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API int32 GetNumOpenPrivateConnections() const;

	/** 可用的公开连接数 */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API int32 GetNumOpenPublicConnections() const;

	/** 可用的最大公开连接数，包括已填充的连接 */
	UFUNCTION(BlueprintPure, Category = Sessions)
	UE_API int32 GetMaxPublicConnections() const;

	/**Ping搜索结果，无法访问MAX_QUERY_Ping */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API int32 GetPingInMs() const;

public:
	/**指向特定于平台的实现的指针 */
#if MODULARUSER_OSSV1
	FOnlineSessionSearchResult Result;
#endif

#if !MODULARUSER_OSSV1
	TSharedPtr<const UE::Online::FLobby> Lobby;

	UE::Online::FOnlineSessionId SessionID;
#endif
};

#undef UE_API
