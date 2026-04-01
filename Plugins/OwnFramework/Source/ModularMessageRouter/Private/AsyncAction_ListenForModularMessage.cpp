// Copyright © 2026 鸿源z. All Rights Reserved.


#include "AsyncAction_ListenForModularMessage.h"

UAsyncAction_ListenForModularMessage* UAsyncAction_ListenForModularMessage::ListenForGameplayMessages(UObject* WorldContextObject, FGameplayTag Channel, UScriptStruct* PayloadType, EModularMessageMatch MatchType)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return nullptr;

	UAsyncAction_ListenForModularMessage* Action = NewObject<UAsyncAction_ListenForModularMessage>();
	Action->CacheWorldPtr = World;
	Action->CacheChannel = Channel;
	Action->CacheMessageStructType = PayloadType;
	Action->CacheMatchType = MatchType;
	Action->RegisterWithGameInstance(World);

	return Action;
}

void UAsyncAction_ListenForModularMessage::Activate()
{
	if (UWorld* World = CacheWorldPtr.Get())
	{
		if (UModularMessageManager::HasInstance(World))
		{
			UModularMessageManager& Router = UModularMessageManager::Get(World);

			TWeakObjectPtr<UAsyncAction_ListenForModularMessage> WeakThis(this);
			CacheListenHandle = Router.RegisterListenerInternal(CacheChannel,
			                                                    [WeakThis](FGameplayTag Channel, const UScriptStruct* StructType, const void* Payload)
			                                                    {
				                                                    if (UAsyncAction_ListenForModularMessage* StrongThis = WeakThis.Get())
					                                                    StrongThis->HandleMessageReceived(Channel, StructType, Payload);
			                                                    },
			                                                    CacheMessageStructType.Get(),
			                                                    CacheMatchType);

			return;
		}
	}

	SetReadyToDestroy();
}

void UAsyncAction_ListenForModularMessage::SetReadyToDestroy()
{
	CacheListenHandle.Unregister();

	Super::SetReadyToDestroy();
}

bool UAsyncAction_ListenForModularMessage::GetPayload(int32& OutPayload)
{
	checkNoEntry();
	return false;
}

DEFINE_FUNCTION(UAsyncAction_ListenForModularMessage::execGetPayload)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* MessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	P_FINISH;

	bool bSuccess = false;

	// 确保我们试图通过蓝图节点获得的类型与接收到的消息有效载荷的类型相匹配。
	if ((StructProp != nullptr) && (StructProp->Struct != nullptr) && (MessagePtr != nullptr) && (StructProp->Struct == P_THIS->CacheMessageStructType.Get()) && (P_THIS->ReceivedMessagePayloadPtr != nullptr))
	{
		StructProp->Struct->CopyScriptStruct(MessagePtr, P_THIS->ReceivedMessagePayloadPtr);
		bSuccess = true;
	}

	*(bool*)RESULT_PARAM = bSuccess;
}

void UAsyncAction_ListenForModularMessage::HandleMessageReceived(FGameplayTag Channel, const UScriptStruct* StructType, const void* Payload)
{
	if (!CacheMessageStructType.Get() || (CacheMessageStructType.Get() == StructType))
	{
		ReceivedMessagePayloadPtr = Payload;

		OnMessageReceived.Broadcast(this, Channel);

		ReceivedMessagePayloadPtr = nullptr;
	}

	if (!OnMessageReceived.IsBound())
	{
		//如果创建异步节点的BP对象被销毁，OnMessageReceived将在调用广播后解除绑定。
		//在这种情况下，我们可以安全地将此接收器标记为准备销毁。
		//需要支持更主动的清理机制FORT-340994
		SetReadyToDestroy();
	}
}
