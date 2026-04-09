// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "ModularSession_SearchSessionRequest.generated.h"

#define UE_API MODULARUSERMANAGEMENT_API

class UModularSession_SearchResult;
enum class EModularSessionOnlineMode : uint8;

/** 会话搜索完成后，广播委托 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FModularSession_FindSessionsFinished, bool bSucceeded, const FText& ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FModularSession_FindSessionsFinishedDynamic, bool, bSucceeded, FText, ErrorMessage);

/**
 * 描述会话搜索的请求对象，搜索完成后将更新此对象
 */
UCLASS(MinimalAPI, Blueprintable)
class UModularSession_SearchSessionRequest : public UObject
{
	GENERATED_BODY()

public:
	/**指示这是在寻找完整的在线游戏还是其他类型的游戏，如局域网*/
	UPROPERTY(BlueprintReadWrite, Category = Session)
	EModularSessionOnlineMode OnlineMode;

	/** 如果此请求应查找玩家托管的大厅（如果可用），则为True，如果为false，则仅搜索已注册的服务器会话 */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUseLobbies;

	/** 调用OnSearchFinished时，所有已找到会话的列表将有效*/
	UPROPERTY(BlueprintReadOnly, Category=Session)
	TArray<TObjectPtr<UModularSession_SearchResult>> Results;

	/** 会话搜索完成时调用本机代表*/
	FModularSession_FindSessionsFinished OnSearchFinished;

	/** 由子系统调用以执行完成的委托 */
	UE_API void NotifySearchFinished(bool bSucceeded, const FText& ErrorMessage);

private:
	/** 会话搜索完成后，广播委托*/
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Search Finished", AllowPrivateAccess = true))
	FModularSession_FindSessionsFinishedDynamic K2_OnSearchFinished;
};

#undef UE_API
