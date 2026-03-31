// Copyright © 2026 鸿源z. All Rights Reserved.


#include "Nodes/AsyncAction_ShowConfirmation.h"

#include "Message/ModularGameDialogDescriptor.h"

UAsyncAction_ShowConfirmation* UAsyncAction_ShowConfirmation::ShowConfirmationYesNo(UObject* InWorldContextObject, FText Title, FText Message)
{
	UAsyncAction_ShowConfirmation* Action = NewObject<UAsyncAction_ShowConfirmation>();
	Action->WorldContextObject = InWorldContextObject;
	Action->Descriptor = UModularGameDialogDescriptor::CreateConfirmationYesNo(Title, Message);
	Action->RegisterWithGameInstance(InWorldContextObject);

	return Action;
}

UAsyncAction_ShowConfirmation* UAsyncAction_ShowConfirmation::ShowConfirmationOkCancel(UObject* InWorldContextObject, FText Title, FText Message)
{
	UAsyncAction_ShowConfirmation* Action = NewObject<UAsyncAction_ShowConfirmation>();
	Action->WorldContextObject = InWorldContextObject;
	Action->Descriptor = UModularGameDialogDescriptor::CreateConfirmationOkCancel(Title, Message);
	Action->RegisterWithGameInstance(InWorldContextObject);

	return Action;
}

UAsyncAction_ShowConfirmation* UAsyncAction_ShowConfirmation::ShowConfirmationCustom(UObject* InWorldContextObject, UModularGameDialogDescriptor* Descriptor)
{
	UAsyncAction_ShowConfirmation* Action = NewObject<UAsyncAction_ShowConfirmation>();
	Action->WorldContextObject = InWorldContextObject;
	Action->Descriptor = Descriptor;
	Action->RegisterWithGameInstance(InWorldContextObject);

	return Action;
}

void UAsyncAction_ShowConfirmation::Activate()
{
	if (WorldContextObject && !TargetLocalPlayer)
	{
		if (UUserWidget* UserWidget = Cast<UUserWidget>(WorldContextObject))
		{
			TargetLocalPlayer = UserWidget->GetOwningLocalPlayer<ULocalPlayer>();
		}
		else if (APlayerController* PC = Cast<APlayerController>(WorldContextObject))
		{
			TargetLocalPlayer = PC->GetLocalPlayer();
		}
		else if (UWorld* World = WorldContextObject->GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance<UGameInstance>())
			{
				TargetLocalPlayer = GameInstance->GetPrimaryPlayerController(false)->GetLocalPlayer();
			}
		}
	}

	if (TargetLocalPlayer)
	{
		if (UModularMessagingSubsystem* Messaging = TargetLocalPlayer->GetSubsystem<UModularMessagingSubsystem>())
		{
			FModularMessagingResultDelegate ResultCallback = FModularMessagingResultDelegate::CreateUObject(this, &UAsyncAction_ShowConfirmation::HandleConfirmationResult);
			Messaging->ShowConfirmation(Descriptor, ResultCallback);
			return;
		}
	}

	// 如果无法确认，则处理未知结果，不进行任何广播。
	HandleConfirmationResult(EModularMessagingResult::Unknown);
}

void UAsyncAction_ShowConfirmation::HandleConfirmationResult(EModularMessagingResult ConfirmationResult)
{
	OnResult.Broadcast(ConfirmationResult);
	SetReadyToDestroy();
}
