// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularLoadingQueryTask.h"

#include "ModularLoadingScreenManager.h"

UModularLoadingQueryTask* UModularLoadingQueryTask::CreateModularLoadingQueryTask(UObject* WorldContextObject, const FString& ShowLoadingScreenReason)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UModularLoadingScreenManager* LoadingScreenManager = GameInstance ? GameInstance->GetSubsystem<UModularLoadingScreenManager>() : nullptr;

	if (LoadingScreenManager)
	{
		UModularLoadingQueryTask* NewLoadingTask = NewObject<UModularLoadingQueryTask>(LoadingScreenManager);
		NewLoadingTask->SetShowLoadingScreenReason(ShowLoadingScreenReason);
		LoadingScreenManager->RegisterLoadingProcessor(NewLoadingTask);

		return NewLoadingTask;
	}

	return nullptr;
}

void UModularLoadingQueryTask::Unregister()
{
	UModularLoadingScreenManager* LoadingScreenManager = Cast<UModularLoadingScreenManager>(GetOuter());
	LoadingScreenManager->UnregisterLoadingProcessor(this);
}

void UModularLoadingQueryTask::SetShowLoadingScreenReason(const FString& InReason)
{
	Reason = InReason;
}

bool UModularLoadingQueryTask::ShouldShowLoadingScreen(FString& OutReason) const
{
	OutReason = Reason;
	return true;
}
