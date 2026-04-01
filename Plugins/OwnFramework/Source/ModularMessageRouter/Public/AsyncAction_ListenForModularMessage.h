// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "ModularMessageManager.h"
#include "ModularMessageTypes.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_ListenForModularMessage.generated.h"

#define UE_API MODULARMESSAGEROUTER_API

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAsyncGameplayMessageDelegate, UAsyncAction_ListenForModularMessage*, ProxyObject, FGameplayTag, ActualChannel);

/**
 * 代理对象引脚将隐藏在K2Node_GameplayMessageAsyncAction中。用于获取触发“GetPayload”后续调用委托的对象的引用。
 * 自定义了一个新的蓝图Node节点,将Payload作为一个输出
 */
UCLASS(MinimalAPI, BlueprintType, meta=(HasDedicatedAsyncNode))
class UAsyncAction_ListenForModularMessage : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * 异步等待指定频道上播放游戏消息。
	 *
	 * @param Channel			要收听的消息频道
	 * @param PayloadType		要使用的消息结构类型（必须与发送方正在广播的类型匹配）
	 * @param MatchType			用于将频道与广播消息匹配的规则
	 */
	UFUNCTION(BlueprintCallable, Category = ModularMessage, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UE_API UAsyncAction_ListenForModularMessage* ListenForGameplayMessages(UObject* WorldContextObject, FGameplayTag Channel, UScriptStruct* PayloadType, EModularMessageMatch MatchType = EModularMessageMatch::ExactMatch);

	UE_API virtual void Activate() override;
	UE_API virtual void SetReadyToDestroy() override;

	/** 当消息在指定频道上广播时调用。使用GetPayload（）请求消息有效负载. */
	UPROPERTY(BlueprintAssignable)
	FAsyncGameplayMessageDelegate OnMessageReceived;

	/**
	 * 尝试将从广播的游戏消息中接收到的负载数据复制并转化到指定的通配符中。
	 * 通配符的类型必须与接收到的消息中的类型匹配。不匹配则转化失败
	 *
	 * @param OutPayload	应将有效载荷复制到通配符引用中
	 * @return				如果复制成功
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = ModularMessage, meta = (CustomStructureParam = "OutPayload"))
	UE_API bool GetPayload(UPARAM(ref) int32& OutPayload);

	DECLARE_FUNCTION(execGetPayload); //在需要通配符的时候,就需要自己定义中间层函数

private:
	void HandleMessageReceived(FGameplayTag Channel, const UScriptStruct* StructType, const void* Payload);

	const void* ReceivedMessagePayloadPtr = nullptr; //负载的任意指针
	TWeakObjectPtr<UWorld> CacheWorldPtr = nullptr;
	FGameplayTag CacheChannel = FGameplayTag();
	TWeakObjectPtr<UScriptStruct> CacheMessageStructType = nullptr;
	EModularMessageMatch CacheMatchType = EModularMessageMatch::ExactMatch;
	FModularMessageListenerHandle CacheListenHandle = FModularMessageListenerHandle();
};
#undef UE_API
