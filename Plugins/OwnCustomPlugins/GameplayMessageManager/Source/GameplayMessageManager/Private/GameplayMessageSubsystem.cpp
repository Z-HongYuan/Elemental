// Copyright © 2026 鸿源z. All Rights Reserved.


#include "GameplayMessageSubsystem.h"


//定义日志分类（在 cpp 文件中实现）
DEFINE_LOG_CATEGORY(LogGameplayMessageSubsystem);

namespace UE
{
	namespace GameplayMessageSubsystem
	{
		static int32 ShouldLogMessages = 0;
		static FAutoConsoleVariableRef CVarShouldLogMessages(TEXT("GameplayMessageSubsystem.LogMessages"),
		                                                     ShouldLogMessages,
		                                                     TEXT("Should messages broadcast through the gameplay message subsystem be logged?"));
	}
}


/*
 * 这里定义了 FGameplayMessageListenerHandle 中的 Unregister 函数
 */
void FGameplayMessageListenerHandle::Unregister()
{
	if (UGameplayMessageSubsystem* StrongSubsystem = Subsystem.Get())
	{
		StrongSubsystem->UnregisterListener(*this);
		Subsystem.Reset();
		Channel = FGameplayTag();
		ID = 0;
	}
}


void UGameplayMessageSubsystem::Deinitialize()
{
	//清理监听列表
	ListenerMap.Reset();

	Super::Deinitialize();
}

UGameplayMessageSubsystem& UGameplayMessageSubsystem::Get(const UObject* WorldContextObject)
{
	//静态的Get函数,确保获取到的UGameplayMessageSubsystem对象有效
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	check(World);
	UGameplayMessageSubsystem* Router = UGameInstance::GetSubsystem<UGameplayMessageSubsystem>(World->GetGameInstance());
	check(Router);
	return *Router;
}

bool UGameplayMessageSubsystem::HasInstance(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	if (!World) return false;
	UGameplayMessageSubsystem* Router = UGameInstance::GetSubsystem<UGameplayMessageSubsystem>(World->GetGameInstance());
	return Router != nullptr;
}

void UGameplayMessageSubsystem::UnregisterListener(FGameplayMessageListenerHandle Handle)
{
	/*
	 * 如果句柄有效且所有权是此子系统，则调用注销句柄的内部操作
	 */
	if (Handle.IsValid())
	{
		check(Handle.Subsystem == this);

		UnregisterListenerInternal(Handle.Channel, Handle.ID);
	}
	else
	{
		UE_LOG(LogGameplayMessageSubsystem, Warning, TEXT("Trying to unregister an invalid Handle."));
	}
}

void UGameplayMessageSubsystem::K2_BroadcastMessage(FGameplayTag Channel, const int32& Message)
{
	// 这个永远不会被调用，相反，下面的exec版本会被调用
	checkNoEntry();
}

DEFINE_FUNCTION(UGameplayMessageSubsystem::execK2_BroadcastMessage)
{
	P_GET_STRUCT(FGameplayTag, Channel);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* MessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (MessagePtr != nullptr)))
	{
		P_THIS->BroadcastMessageInternal(Channel, StructProp->Struct, MessagePtr);
	}
}

void UGameplayMessageSubsystem::BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes)
{
	// 如果启用，则记录消息
	if (UE::GameplayMessageSubsystem::ShouldLogMessages != 0)
	{
		FString* pContextString = nullptr;
#if WITH_EDITOR
		if (GIsEditor)
		{
			extern ENGINE_API FString GPlayInEditorContextString;
			pContextString = &GPlayInEditorContextString;
		}
#endif

		FString HumanReadableMessage;
		StructType->ExportText(/*out*/ HumanReadableMessage, MessageBytes, /*Defaults=*/ nullptr, /*OwnerObject=*/ nullptr, PPF_None, /*ExportRootScope=*/ nullptr);
		UE_LOG(LogGameplayMessageSubsystem, Log, TEXT("BroadcastMessage(%s, %s, %s)"), pContextString ? **pContextString : *GetPathNameSafe(this), *Channel.ToString(),
		       *HumanReadableMessage);
	}

	/*
	 * 接下来就是广播消息
	 */

	//标记是否是第一次迭代（精确匹配的 Tag）
	bool bOnInitialTag = true;

	// 假设广播的 Tag 是："Game.Player.ScoreChanged"
	// 第 1 次迭代：Tag = "Game.Player.ScoreChanged" (自己)
	// 第 2 次迭代：Tag = "Game.Player"          (父级)
	// 第 3 次迭代：Tag = "Game"                 (祖父级)
	// 第 4 次迭代：Tag = ""                     (无效，循环结束)
	//Tag 层级遍历 从当前Tag开始请求父级标签,直到标签无效
	for (FGameplayTag Tag = Channel; Tag.IsValid(); Tag = Tag.RequestDirectParent())
	{
		//找到监听此 Tag 的监听器
		if (const FChannelListenerList* pList = ListenerMap.Find(Tag))
		{
			// 以防在处理回调时发生移除，请进行复制
			/*
			ListenerMap:
			┌──────────────────────┬─────────────────────┐
			│ Tag                  │ Listeners           │
			├──────────────────────┼─────────────────────┤
			│ "Game.Player"        │ [Listener1, ...]    │
			│ "Game.Player.Score"  │ [Listener2, ...]    │
			│ "Game.Enemy"         │ [Listener3, ...]    │
			└──────────────────────┴─────────────────────┘
			 */
			TArray<FGameplayMessageListenerData> ListenerArray(pList->Listeners);

			//从复制的容器中处理每一个监听器的回调
			for (const FGameplayMessageListenerData& Listener : ListenerArray)
			{
				//逻辑判断 部分匹配或者第一次迭代(第一次迭代代表着跟自己一样的Tag)
				if (bOnInitialTag || (Listener.MatchType == EGameplayMessageMatchType::PartialMatch))
				{
					//类型有效性检查 如果无效就移除监听器
					if (Listener.bHadValidType && !Listener.ListenerStructType.IsValid())
					{
						UE_LOG(LogGameplayMessageSubsystem, Warning, TEXT("Listener struct type has gone invalid on Channel %s. Removing listener from list"), *Channel.ToString());
						UnregisterListenerInternal(Channel, Listener.HandleID);
						continue; // 跳过本次迭代，处理下一个监听器
					}

					// 接收类型必须是发送类型的父类，或者完全不明确（供C++） 类型兼容性检查
					// Listener.bHadValidType 如果监听器注册时没有指定类型（nullptr），则接受任何类型,用于内部系统或通用处理器
					if (!Listener.bHadValidType || StructType->IsChildOf(Listener.ListenerStructType.Get()))
					{
						Listener.ReceivedCallback(Channel, StructType, MessageBytes);
					}
					else
					{
						//本来这里是使用的 Error 类型,但是设想一下,同一个Tag有多个不同的结构体进行监听,这样的话 Error 打印的Log就比较乱的
						UE_LOG(LogGameplayMessageSubsystem, Warning, TEXT("Struct type mismatch on channel %s (broadcast type %s, listener at %s was expecting type %s)"),
						       *Channel.ToString(),
						       *StructType->GetPathName(),
						       *Tag.ToString(),
						       *Listener.ListenerStructType->GetPathName());
					}
				}
			}
		}
		// 第一次迭代后设为 false 后续迭代只触发 PartialMatch(部分匹配) 的监听器
		bOnInitialTag = false;
	}
}

FGameplayMessageListenerHandle UGameplayMessageSubsystem::RegisterListenerInternal(FGameplayTag Channel,
                                                                                   TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
                                                                                   const UScriptStruct* StructType, EGameplayMessageMatchType MatchType)
{
	/*
	 * 使用方便的结构体传递参数,并且返回一个句柄用于后期操作
	 * 就是构建一个句柄并返回
	 */

	FChannelListenerList& List = ListenerMap.FindOrAdd(Channel);

	FGameplayMessageListenerData& Entry = List.Listeners.AddDefaulted_GetRef();
	Entry.ReceivedCallback = MoveTemp(Callback);
	Entry.ListenerStructType = StructType;
	Entry.bHadValidType = StructType != nullptr;
	Entry.HandleID = ++List.HandleID;
	Entry.MatchType = MatchType;

	return FGameplayMessageListenerHandle(this, Channel, Entry.HandleID);
}

void UGameplayMessageSubsystem::UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID)
{
	/*
	 * 从监听器列表中移除指定的监听器
	 * 查找方式是通过Tag和句柄中包含的HandID
	 */
	if (FChannelListenerList* pList = ListenerMap.Find(Channel))
	{
		//这里使用查询函数,通过简短的函数判断来查询对应ID的监听器
		int32 MatchIndex = pList->Listeners.IndexOfByPredicate([ID = HandleID](const FGameplayMessageListenerData& Other) { return Other.HandleID == ID; });
		if (MatchIndex != INDEX_NONE)
		{
			pList->Listeners.RemoveAtSwap(MatchIndex);
		}

		if (pList->Listeners.Num() == 0)
		{
			ListenerMap.Remove(Channel);
		}
	}
}
