// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayMessageTypes.generated.h"

#define UE_API GAMEPLAYMESSAGEMANAGER_API

class UGameplayMessageSubsystem;

// 用于判断接受回调的匹配规则
UENUM(BlueprintType)
enum class EGameplayMessageMatchType : uint8
{
	/*
	 * 全匹配/完全匹配
	 * 只允许 A.B == A.B 或者 A.B.C == A.B.C
	 */
	ExactMatch,

	/*
	 * 部分匹配
	 * 允许 A.B == A.B.C 和 A.B == A.B
	 * 当前Tag为匹配Tag的子集
	 */
	PartialMatch
};

/**
 * 用于在注册游戏消息监听器时使用的结构体
 */
template <typename FMessageStructType>
struct FGameplayMessageListenerParams
{
	/*
	 * 匹配的规则
	 * 详情参见 EGameplayMessageMatch
	 */
	EGameplayMessageMatchType MatchType = EGameplayMessageMatchType::ExactMatch;

	/*
	 * 绑定的回调函数,当广播被触发时,此函数会被调用
	 */
	TFunction<void(FGameplayTag, const FMessageStructType&)> OnMessageReceivedCallback;

	/*
	 * 工具函数
	 * 用于将弱成员函数绑定到 OnMessageReceivedCallback
	 */
	template <typename TOwner = UObject>
	void SetMessageReceivedCallback(TOwner* Object, void (TOwner::*Function)(FGameplayTag, const FMessageStructType&))
	{
		TWeakObjectPtr<TOwner> WeakObject(Object);
		OnMessageReceivedCallback = [WeakObject, Function](FGameplayTag Channel, const FMessageStructType& Payload)
		{
			if (TOwner* StrongObject = WeakObject.Get())
			{
				(StrongObject->*Function)(Channel, Payload);
			}
		};
	}
};

/*
 * 一个不透明的句柄，可用于移除之前注册的消息监听器,意味着在使用结构体注册了监听器之后,返回的一个控制句柄
 * 一般会保存下来，以便在需要时使用
 * 用于使用 UGameplayMessageSubsystem::RegisterListener 和 UGameplayMessageSubsystem::UnregisterListener 函数
 */
USTRUCT(BlueprintType)
struct FGameplayMessageListenerHandle
{
public:
	GENERATED_BODY()

	FGameplayMessageListenerHandle()
	{
	}

	UE_API void Unregister();

	bool IsValid() const { return ID != 0; }

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UGameplayMessageSubsystem> Subsystem;

	UPROPERTY(Transient)
	FGameplayTag Channel;

	UPROPERTY(Transient)
	int32 ID = 0;

	FDelegateHandle StateClearedHandle;

	friend UGameplayMessageSubsystem;

	FGameplayMessageListenerHandle(UGameplayMessageSubsystem* InSubsystem, FGameplayTag InChannel, int32 InID) : Subsystem(InSubsystem), Channel(InChannel), ID(InID)
	{
	}
};

/** 
 * 单个已注册监听器的条目信息
 * 用在Subsystem中的内部数据
 */
USTRUCT()
struct FGameplayMessageListenerData
{
	GENERATED_BODY()

	// 消息接收时的回调函数
	TFunction<void(FGameplayTag, const UScriptStruct*, const void*)> ReceivedCallback;

	int32 HandleID;
	EGameplayMessageMatchType MatchType;

	// 针对此处的某些潜在问题，增加一些日志记录和额外变量
	//这里保存的时候结构体特征量的引用
	TWeakObjectPtr<const UScriptStruct> ListenerStructType = nullptr;
	bool bHadValidType = false;
};

#undef UE_API
