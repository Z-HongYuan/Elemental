// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayMessageTypes.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayMessageSubsystem.generated.h"

#define UE_API GAMEPLAYMESSAGEMANAGER_API

// 声明当前模块的日志分类（在头文件中声明）
DECLARE_LOG_CATEGORY_EXTERN(LogGameplayMessageSubsystem, Log, All);

/**
 * 该系统允许事件引发者和监听者在不直接相互了解的情况下注册消息，但它们必须就消息的格式（作为USTRUCT()类型）达成一致。
 * 只要能获取到此子系统,就能发送和接收消息
 * 请注意，当同一通道有多个监听器时，调用顺序无法保证，并且可能会随时间而变化
 */
UCLASS(MinimalAPI)
class UGameplayMessageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	friend class UAsyncAction_ListenForGameplayMessage;

	//~USubsystem interface
	UE_API virtual void Deinitialize() override;
	//~End of USubsystem interface

	// 获取当前世界的消息子系统
	static UE_API UGameplayMessageSubsystem& Get(const UObject* WorldContextObject);

	// 如果当前世界中存在一个有效的消息子系统，则返回true
	static UE_API bool HasInstance(const UObject* WorldContextObject);

	/**
	 * 在指定频道上广播条消息
	 * 这里是一个模版函数,用于允许使用模版参数来指定消息结构
	 *
	 * @param Channel			要广播的消息通道(使用GameplayTag)
	 * @param Message			要发送的消息（必须与此通道的监听器所期望的UScriptStruct类型相同，否则将记录错误）
	 */
	template <typename FMessageStructType>
	void BroadcastMessage(FGameplayTag Channel, const FMessageStructType& Message)
	{
		/*
		 * TBaseStructure 是什么？
		 * Unreal 的类型 traits 系统
		 * 将 C++ 类型映射到引擎的反射系统
		 * 只有 USTRUCT() 标记的结构体才能使用
		 * 代表用户能够自定义蓝图结构体
		 */
		const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
		BroadcastMessageInternal(Channel, StructType, &Message);
	}

	/**
	 * 在对应Tag上注册一个监听器
	 * 模版函数,拥有函数参数,能够传递函数引用来调用
	 *
	 * @param Channel			要监听的Tag
	 * @param Callback			当有人广播消息时，要调用的函数（该函数必须与该频道广播者提供的UScriptStruct类型相同，否则将记录错误）
	 * @param MatchType         匹配类型
	 *
	 * @return 一个可用于注销此侦听器的句柄（可以通过对该句柄调用 Unregister() 方法或对路由器调用 UnregisterListener 方法来实现）
	 */
	template <typename FMessageStructType>
	FGameplayMessageListenerHandle RegisterListener(FGameplayTag Channel, TFunction<void(FGameplayTag, const FMessageStructType&)>&& Callback /*Callback（右值引用回调函数）*/,
	                                                EGameplayMessageMatchType MatchType = EGameplayMessageMatchType::ExactMatch)
	{
		//Thunk 回调包装器 类型擦除适配器
		auto ThunkCallback = [InnerCallback = MoveTemp(Callback)](FGameplayTag ActualTag, const UScriptStruct* SenderStructType, const void* SenderPayload)
		{
			InnerCallback(ActualTag, *reinterpret_cast<const FMessageStructType*>(SenderPayload));
		};

		const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
		return RegisterListenerInternal(Channel, ThunkCallback, StructType, MatchType);
	}


	/**
	 * 注册以接收指定通道上的消息，并使用指定的成员函数处理该消息
	 * 在触发回调之前，执行一次弱对象有效性检查，以确保注册该函数的对象仍然存在
	 *
	 * @param Channel			监听的Tag通道
	 * @param Object			要调用函数的对象实例
	 * @param Function			当有人广播消息时，使用该消息调用的成员函数（必须与该频道广播者提供的UScriptStruct类型相同，否则将记录错误）
	 *
	 * @return 一个可用于注销此侦听器的句柄（可以通过对该句柄调用 Unregister() 方法或对路由器调用 UnregisterListener 方法来实现）
	 */
	template <typename FMessageStructType, typename TOwner = UObject>
	FGameplayMessageListenerHandle RegisterListener(FGameplayTag Channel, TOwner* Object, void (TOwner::*Function)(FGameplayTag, const FMessageStructType&))
	{
		TWeakObjectPtr<TOwner> WeakObject(Object);
		return RegisterListener<FMessageStructType>(Channel,
		                                            [WeakObject, Function](FGameplayTag Channel, const FMessageStructType& Payload)
		                                            {
			                                            if (TOwner* StrongObject = WeakObject.Get())
			                                            {
				                                            (StrongObject->*Function)(Channel, Payload);
			                                            }
		                                            });
	}

	/**
	 * 注册以在指定通道上接收消息，并附带额外参数以支持高级行为
	 * 这个逻辑中的有状态部分或许应该分离出来，放到一个单独的系统中
	 *
	 * @param Channel			监听的Tag通道
	 * @param Params			包含高级行为细节的结构
	 *
	 * @return 一个可用于注销此侦听器的句柄（可通过在该句柄上调用 Unregister() 或在路由器上调用 UnregisterListener 来实现）
	 */
	template <typename FMessageStructType>
	FGameplayMessageListenerHandle RegisterListener(FGameplayTag Channel, FGameplayMessageListenerParams<FMessageStructType>& Params)
	{
		FGameplayMessageListenerHandle Handle;

		// 注册以接收此频道未来发布的任何消息
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
	 * 移除之前由RegisterListener注册的消息监听器
	 *
	 * @param Handle	RegisterListener返回的句柄
	 */
	UE_API void UnregisterListener(FGameplayMessageListenerHandle Handle);

protected:
	/**
	 * 在指定频道上发布一条消息
	 *
	 * @param Channel			广播的Tag通道
	 * @param Message			要发送的消息（必须与此通道的监听器所期望的UScriptStruct类型相同，否则将记录错误）
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category=Messaging, meta=(CustomStructureParam="Message", AllowAbstract="false", DisplayName="Broadcast Message"))
	UE_API void K2_BroadcastMessage(FGameplayTag Channel, const int32& Message);

	DECLARE_FUNCTION(execK2_BroadcastMessage);

private:
	// 用于广播消息的内部助手 向所有注册了指定 Tag 通道的监听器广播消息
	UE_API void BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes);

	// 用于注册消息监听器的内部助手
	UE_API FGameplayMessageListenerHandle RegisterListenerInternal(
		FGameplayTag Channel,
		TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
		const UScriptStruct* StructType,
		EGameplayMessageMatchType MatchType);

	UE_API void UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID);

private:
	// 简单结构体, 用于存储监听器
	struct FChannelListenerList
	{
		// 该通道的所有监听器
		TArray<FGameplayMessageListenerData> Listeners;
		
		// 递增的ID生成器
		int32 HandleID = 0;
	};

private:
	//最终存储监听器的容器
	TMap<FGameplayTag, FChannelListenerList> ListenerMap;
};

#undef UE_API
