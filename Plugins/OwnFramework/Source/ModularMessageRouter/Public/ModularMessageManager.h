// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "ModularMessageTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ModularMessageManager.generated.h"

#define UE_API MODULARMESSAGEROUTER_API

class UModularMessageManager;
struct FGameplayTag;

/*一个句柄,一般用于删除监听器*/
USTRUCT(BlueprintType)
struct FModularMessageListenerHandle
{
public:
	GENERATED_BODY()

	FModularMessageListenerHandle()
	{
	}

	UE_API void Unregister();

	bool IsValid() const { return ID != 0; }

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UModularMessageManager> Subsystem;

	UPROPERTY(Transient)
	FGameplayTag Channel;

	UPROPERTY(Transient)
	int32 ID = 0;

	FDelegateHandle StateClearedHandle;

	friend UModularMessageManager;

	FModularMessageListenerHandle(UModularMessageManager* InSubsystem, FGameplayTag InChannel, int32 InID) : Subsystem(InSubsystem), Channel(InChannel), ID(InID)
	{
	}
};


/**
 *该系统允许事件发起者和听众注册消息，而无需
 *必须直接了解对方，尽管他们必须就格式达成一致
 *消息的（作为USTRUCT（）类型）。
 *
 *您可以从游戏实例访问消息路由器：
 *UGameInstance：：GetSubsystem＜UGameplayMessageSubsystem＞（游戏实例）
 *或者直接来自任何有世界上下文的东西：
 *UGameplayMessageSubsystem：：获取（WorldContextObject）
 *
 *请注意，当同一频道有多个监听器时，调用顺序为
 *不保证，会随着时间的推移而改变！
 */
UCLASS(MinimalAPI)
class UModularMessageManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	UE_API virtual void Deinitialize() override;
	//~End of USubsystem interface

	//返回与指定对象的世界关联的游戏实例的消息路由器
	static UE_API UModularMessageManager& Get(const UObject* WorldContextObject);

	//在世界中是否存在有效的消息路由器
	static UE_API bool HasInstance(const UObject* WorldContextObject);

	/**
	 * 在指定频道上广播消息
	 *
	 * @param Channel			要广播的消息频道
	 * @param Message			要发送的消息（必须与此通道的侦听器预期的UScriptStruct类型相同，否则将记录错误）
	 */
	template <typename FMessageStructType>
	void BroadcastMessage(FGameplayTag Channel, const FMessageStructType& Message)
	{
		const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
		BroadcastMessageInternal(Channel, StructType, &Message);
	}

	/**
	 * 注册以在指定频道上接收消息
	 *
	 * @param Channel			要监听的消息频道
	 * @param Callback			当有人广播消息时调用该函数（必须与此通道的侦听器预期的UScriptStruct类型相同，否则将记录错误）
	 * @param MatchType			匹配规则
	 *
	 * @return 一个可用于注销此侦听器的句柄（通过在句柄上调用 unregister（）或在管理器上调用 UnreegisterListener）
	 */
	template <typename FMessageStructType>
	FModularMessageListenerHandle RegisterListener(FGameplayTag Channel, TFunction<void(FGameplayTag, const FMessageStructType&)>&& Callback, EModularMessageMatch MatchType = EModularMessageMatch::ExactMatch)
	{
		//使用匿名函数,内部转换函数接口,使其统一且兼容
		auto ThunkCallback = [InnerCallback = MoveTemp(Callback)](FGameplayTag ActualTag, const UScriptStruct* SenderStructType, const void* SenderPayload)
		{
			InnerCallback(ActualTag, *reinterpret_cast<const FMessageStructType*>(SenderPayload));
		};

		const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
		return RegisterListenerInternal(Channel, ThunkCallback, StructType, MatchType);
	}

	/**
	 * 注册以在指定通道上接收消息，并使用指定的成员函数对其进行处理
	 * 执行弱对象有效性检查，以确保在触发回调之前注册函数的对象仍然存在
	 *
	 * @param Channel			要监听的消息频道
	 * @param Object			成员函数的成员对象
	 * @param Function			成员函数，用于在有人广播消息时调用消息（必须与此通道的侦听器预期的UScriptStruct类型相同，否则将记录错误）
	 *
	 * @return 一个可用于注销此侦听器的句柄（通过在句柄上调用unregister（）或在路由器上调用UnreegisterListener）
	 */
	template <typename FMessageStructType, typename TOwner = UObject>
	FModularMessageListenerHandle RegisterListener(FGameplayTag Channel, TOwner* Object, void (TOwner::*Function)(FGameplayTag, const FMessageStructType&))
	{
		//使用匿名函数,内部绑定成员函数,使其统一且兼容
		TWeakObjectPtr<TOwner> WeakObject(Object);
		return RegisterListener<FMessageStructType>(Channel,
		                                            [WeakObject, Function](FGameplayTag Channel, const FMessageStructType& Payload)
		                                            {
			                                            if (TOwner* StrongObject = WeakObject.Get())
				                                            (StrongObject->*Function)(Channel, Payload);
		                                            });
	}

	/**
	 * 注册以在指定通道上接收消息，并使用额外参数支持高级行为
	 * 这个逻辑的有状态部分可能应该分离到一个单独的系统中
	 *
	 * @param Channel			要监听的消息频道
	 * @param Params			包含高级行为详细信息的结构体
	 *
	 * @return 一个可用于注销此侦听器的句柄（通过在句柄上调用unregister（）或在路由器上调用 UnreegisterListener）
	 */
	template <typename FMessageStructType>
	FModularMessageListenerHandle RegisterListener(FGameplayTag Channel, FModularMessageListenerParams<FMessageStructType>& Params)
	{
		FModularMessageListenerHandle Handle;

		//如果回调有效才注册
		if (Params.OnMessageReceivedCallback)
		{
			auto ThunkCallback = [InnerCallback = Params.OnMessageReceivedCallback](FGameplayTag ActualTag, const UScriptStruct* SenderStructType, const void* SenderPayload)
			{
				InnerCallback(ActualTag, *reinterpret_cast<const FMessageStructType*>(SenderPayload));
			};

			const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
			Handle = RegisterListenerInternal(Channel, ThunkCallback, StructType, Params.MatchType);
		}

		return Handle;
	}

	/**
	 * 删除以前由RegisterListener注册的消息侦听器
	 *
	 * @param Handle	监听器句柄
	 */
	UE_API void UnregisterListener(FModularMessageListenerHandle Handle);

protected:
	/**
	 * 在指定频道上广播消息
	 *
	 * @param Channel			要广播的消息频道
	 * @param Message			要发送的消息（必须与此通道的侦听器预期的UScriptStruct类型相同，否则将记录错误）
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category=ModularMessage, meta=(CustomStructureParam="Message", AllowAbstract="false", DisplayName="Broadcast Message"))
	UE_API void K2_BroadcastMessage(FGameplayTag Channel, const int32& Message);

	DECLARE_FUNCTION(execK2_BroadcastMessage); //在需要通配符的时候,就需要自己定义中间层函数

private:
	//内部使用的广播函数
	UE_API void BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes);

	// 内部使用的注册函数
	UE_API FModularMessageListenerHandle RegisterListenerInternal(
		FGameplayTag Channel,
		TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
		const UScriptStruct* StructType,
		EModularMessageMatch MatchType);

	//内部使用的注销函数
	UE_API void UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID);

	//友元,方便异步节点访问
	friend class UAsyncAction_ListenForModularMessage;

	// 每个Tag代表的所有监听器
	struct FChannelListenerList
	{
		TArray<FModularMessageListenerData> Listeners;
		int32 HandleID = 0;
	};

	TMap<FGameplayTag, FChannelListenerList> ListenerMap;
};

#undef UE_API
