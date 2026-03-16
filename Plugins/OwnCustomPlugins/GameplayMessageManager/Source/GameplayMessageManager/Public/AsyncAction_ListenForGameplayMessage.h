// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayMessageTypes.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_ListenForGameplayMessage.generated.h"

#define UE_API GAMEPLAYMESSAGEMANAGER_API

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAsyncGameplayMessageDelegate, UAsyncAction_ListenForGameplayMessage*, ProxyObject, FGameplayTag, ActualChannel);

/**
 * 
 */
UCLASS(MinimalAPI, BlueprintType, meta=(HasDedicatedAsyncNode))
class UAsyncAction_ListenForGameplayMessage : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * 异步等待在指定频道上播放游戏消息。
	 *
	 * @param Channel			要监听的消息通道
	 * @param PayloadType		要使用的消息结构类型（必须与发送者广播的消息类型相匹配）
	 * @param MatchType			用于将频道与已广播消息进行匹配的规则
	 */
	UFUNCTION(BlueprintCallable, Category = Messaging, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UE_API UAsyncAction_ListenForGameplayMessage* ListenForGameplayMessages(UObject* WorldContextObject, FGameplayTag Channel, UScriptStruct* PayloadType,
	                                                                               EGameplayMessageMatchType MatchType = EGameplayMessageMatchType::ExactMatch);

	/**
	 * 尝试将从广播的游戏消息中接收到的有效载荷复制到指定的通配符中。
	 * 通配符的类型必须与接收到的消息中的类型相匹配
	 *
	 * @param OutPayload	应将通配符引用复制到有效载荷中
	 * @return				如果复制成功
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Messaging", meta = (CustomStructureParam = "OutPayload"))
	UE_API bool GetPayload(UPARAM(ref) int32& OutPayload);

	DECLARE_FUNCTION(execGetPayload);

	UE_API virtual void Activate() override;
	UE_API virtual void SetReadyToDestroy() override;

public:
	/** 当消息在指定通道上广播时调用。使用 GetPayload() 请求消息有效负载。 */
	UPROPERTY(BlueprintAssignable)
	FAsyncGameplayMessageDelegate OnMessageReceived;

private:
	void HandleMessageReceived(FGameplayTag Channel, const UScriptStruct* StructType, const void* Payload);

private:
	const void* ReceivedMessagePayloadPtr = nullptr;

	TWeakObjectPtr<UWorld> WorldPtr;
	FGameplayTag ChannelToRegister;
	TWeakObjectPtr<UScriptStruct> MessageStructType = nullptr;
	EGameplayMessageMatchType MessageMatchType = EGameplayMessageMatchType::ExactMatch;

	FGameplayMessageListenerHandle ListenerHandle;
};

#undef UE_API
