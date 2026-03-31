// Copyright © 2026 鸿源z. All Rights Reserved.


#include "Message/ModularMessagingSubsystem.h"

void UModularMessagingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UModularMessagingSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UModularMessagingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	UGameInstance* GameInstance = CastChecked<ULocalPlayer>(Outer)->GetGameInstance();
	if (GameInstance && !GameInstance->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		// 仅当其他地方没有定义重写实现时才创建实例。
		return ChildClasses.Num() == 0;
	}

	return false;
}

void UModularMessagingSubsystem::ShowConfirmation(class UModularGameDialogDescriptor* DialogDescriptor, FModularMessagingResultDelegate ResultCallback)
{
}

void UModularMessagingSubsystem::ShowError(class UModularGameDialogDescriptor* DialogDescriptor, FModularMessagingResultDelegate ResultCallback)
{
}
