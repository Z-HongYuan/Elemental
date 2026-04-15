// Copyright © 2026 鸿源z. All Rights Reserved.


#include "SystemMessage/ModularMessagingManager.h"

#include "ModularUITags.h"
#include "Widgets/ModularGameDialog.h"
#include "Widgets/ModularRootLayoutWidget.h"


bool UModularMessagingManager::ShouldCreateSubsystem(UObject* Outer) const
{
	//保证继承链单例
	UGameInstance* GameInstance = CastChecked<ULocalPlayer>(Outer)->GetGameInstance();
	if (GameInstance && !GameInstance->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		return ChildClasses.Num() == 0;
	}

	return false;
}

void UModularMessagingManager::ShowConfirmation(UModularGameDialogDataObj* DialogDataObj, FModularMessagingResultDelegate ResultCallback)
{
}

void UModularMessagingManager::ShowError(UModularGameDialogDataObj* DialogDataObj, FModularMessagingResultDelegate ResultCallback)
{
}
