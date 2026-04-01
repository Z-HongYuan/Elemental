// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "ModularMessageTypes.generated.h"

#define UE_API MODULARMESSAGEROUTER_API

//消息侦听器的匹配规则
UENUM(BlueprintType)
enum class EModularMessageMatch : uint8
{
	//精确匹配只会接收具有完全相同通道的消息
	//（例如，注册“A.B”将匹配A.B的广播，但不匹配A.B.C的广播）
	ExactMatch,

	//部分匹配将接收所有子类于同一通道的任何消息
	//（例如，注册“A.B”将匹配A.B和A.B.C的广播）
	PartialMatch
};

//结构用于在注册游戏消息的侦听器时指定高级行为
template <typename FMessageStructType>
struct FModularMessageListenerParams
{
	//Tag的匹配规则
	EModularMessageMatch MatchType = EModularMessageMatch::ExactMatch;

	//消息被触发时的调用回调/消息收到后触发的函数
	TFunction<void(FGameplayTag, const FMessageStructType&)> OnMessageReceivedCallback;

	/**将弱成员函数绑定到 OnMessageReceivedCallback 的帮助函数*/
	template <typename TOwner = UObject>
	void SetMessageReceivedCallback(TOwner* Object, void (TOwner::*Function)(FGameplayTag, const FMessageStructType&))
	{
		TWeakObjectPtr<TOwner> WeakObject(Object);
		OnMessageReceivedCallback = [WeakObject, Function](FGameplayTag Channel, const FMessageStructType& Payload)
		{
			if (TOwner* StrongObject = WeakObject.Get())
				(StrongObject->*Function)(Channel, Payload);
		};
	}
};


/* 以下原本属于Manager的.h文件内 */

/*用于注册单个监听器的数据结构*/
USTRUCT()
struct FModularMessageListenerData
{
	GENERATED_BODY()

	// 收到消息时的回调
	TFunction<void(FGameplayTag, const UScriptStruct*, const void*)> ReceivedCallback;

	int32 HandleID;
	EModularMessageMatch MatchType;

	// 围绕一些潜在问题添加一些日志记录和额外变量
	TWeakObjectPtr<const UScriptStruct> ListenerStructType = nullptr;
	bool bHadValidType = false;
};

#undef UE_API
