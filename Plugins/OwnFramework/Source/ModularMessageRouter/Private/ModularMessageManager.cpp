// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularMessageRouter/Public/ModularMessageManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogModularMessageManager, Log, All); //注册日志
DEFINE_LOG_CATEGORY(LogModularMessageManager); //注册日志

namespace UE
{
	namespace ModularMessageManager
	{
		static int32 ShouldLogMessages = 0;
		static FAutoConsoleVariableRef CVarShouldLogMessages(TEXT("ModularMessageManager.LogMessages"),
		                                                     ShouldLogMessages,
		                                                     TEXT("Should messages broadcast through the modular message Manager be logged?"));
	}
}

void FModularMessageListenerHandle::Unregister()
{
	if (UModularMessageManager* StrongSubsystem = Subsystem.Get())
	{
		StrongSubsystem->UnregisterListener(*this);
		Subsystem.Reset();
		Channel = FGameplayTag();
		ID = 0;
	}
}

void UModularMessageManager::Deinitialize()
{
	ListenerMap.Reset();
	Super::Deinitialize();
}

UModularMessageManager& UModularMessageManager::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	check(World);
	UModularMessageManager* Router = UGameInstance::GetSubsystem<UModularMessageManager>(World->GetGameInstance());
	check(Router);
	return *Router;
}

bool UModularMessageManager::HasInstance(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	UModularMessageManager* Router = World != nullptr ? UGameInstance::GetSubsystem<UModularMessageManager>(World->GetGameInstance()) : nullptr;
	return Router != nullptr;
}

void UModularMessageManager::UnregisterListener(FModularMessageListenerHandle Handle)
{
	if (Handle.IsValid())
	{
		check(Handle.Subsystem == this);

		UnregisterListenerInternal(Handle.Channel, Handle.ID);
	}
	else
	{
		UE_LOG(LogModularMessageManager, Warning, TEXT("Trying to unregister an invalid Handle."));
	}
}

void UModularMessageManager::K2_BroadcastMessage(FGameplayTag Channel, const int32& Message)
{
	// 这将永远不会被调用，应该是调用 UModularMessageManager::execK2_BroadcastMessage函数
	checkNoEntry();
}

DEFINE_FUNCTION(UModularMessageManager::execK2_BroadcastMessage)
{
	P_GET_STRUCT(FGameplayTag, Channel);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* MessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (MessagePtr != nullptr)))
		P_THIS->BroadcastMessageInternal(Channel, StructProp->Struct, MessagePtr);
}

void UModularMessageManager::BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes)
{
	// 如果启用，则记录消息内容到日志
	if (UE::ModularMessageManager::ShouldLogMessages != 0)
	{
		FString* ContextString = nullptr;
#if WITH_EDITOR
		if (GIsEditor)
		{
			extern ENGINE_API FString GPlayInEditorContextString;
			ContextString = &GPlayInEditorContextString;
		}
#endif

		FString HumanReadableMessage;
		StructType->ExportText(/*out*/ HumanReadableMessage, MessageBytes, /*Defaults=*/ nullptr, /*OwnerObject=*/ nullptr, PPF_None, /*ExportRootScope=*/ nullptr);
		UE_LOG(LogModularMessageManager, Log, TEXT("BroadcastMessage(%s, %s, %s)"), ContextString ? **ContextString : *GetPathNameSafe(this), *Channel.ToString(), *HumanReadableMessage);
	}

	// 广播消息
	bool bOnInitialTag = true;
	for (FGameplayTag Tag = Channel; Tag.IsValid(); Tag = Tag.RequestDirectParent())
	{
		if (const FChannelListenerList* ListenList = ListenerMap.Find(Tag))
		{
			// 在处理回调时进行复制，防止删除后会空指针悬挂
			TArray<FModularMessageListenerData> ListenerArray(ListenList->Listeners);

			for (const FModularMessageListenerData& Listener : ListenerArray)
			{
				if (bOnInitialTag || (Listener.MatchType == EModularMessageMatch::PartialMatch))
				{
					if (Listener.bHadValidType && !Listener.ListenerStructType.IsValid())
					{
						UE_LOG(LogModularMessageManager, Warning, TEXT("Listener struct type has gone invalid on Channel %s. Removing listener from list"), *Channel.ToString());
						UnregisterListenerInternal(Channel, Listener.HandleID);
						continue;
					}

					// 处理回调时发生删除时进行复制接收类型必须是发送类型的父类型或完全不明确（供内部使用）
					if (!Listener.bHadValidType || StructType->IsChildOf(Listener.ListenerStructType.Get()))
					{
						Listener.ReceivedCallback(Channel, StructType, MessageBytes);
					}
					else
					{
						UE_LOG(LogModularMessageManager, Error, TEXT("Struct type mismatch on channel %s (broadcast type %s, listener at %s was expecting type %s)"),
						       *Channel.ToString(),
						       *StructType->GetPathName(),
						       *Tag.ToString(),
						       *Listener.ListenerStructType->GetPathName());
					}
				}
			}
		}
		bOnInitialTag = false;
	}
}

FModularMessageListenerHandle UModularMessageManager::RegisterListenerInternal(FGameplayTag Channel, TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback, const UScriptStruct* StructType,
                                                                               EModularMessageMatch MatchType)
{
	FChannelListenerList& List = ListenerMap.FindOrAdd(Channel);

	FModularMessageListenerData& Entry = List.Listeners.AddDefaulted_GetRef();
	Entry.ReceivedCallback = MoveTemp(Callback);
	Entry.ListenerStructType = StructType;
	Entry.bHadValidType = StructType != nullptr;
	Entry.HandleID = ++List.HandleID;
	Entry.MatchType = MatchType;

	return FModularMessageListenerHandle(this, Channel, Entry.HandleID);
}

void UModularMessageManager::UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID)
{
	if (FChannelListenerList* ListenList = ListenerMap.Find(Channel))
	{
		const int32 MatchIndex = ListenList->Listeners.IndexOfByPredicate([ID = HandleID](const FModularMessageListenerData& Other) { return Other.HandleID == ID; });

		//索引有效就删除对应的监听器
		if (MatchIndex != INDEX_NONE)
			ListenList->Listeners.RemoveAtSwap(MatchIndex);

		//索引无效就直接删除对应的Tag频道
		if (ListenList->Listeners.Num() == 0)
			ListenerMap.Remove(Channel);
	}
}
