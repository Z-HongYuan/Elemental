// Copyright © 2026 鸿源z. All Rights Reserved.


#include "PocketWorlds/Public/PocketLevelInstance.h"

#include "Engine/LevelStreamingDynamic.h"
#include "PocketWorlds/Public/PocketLevelAsset.h"

UPocketLevelInstance::UPocketLevelInstance()
{
}

void UPocketLevelInstance::BeginDestroy()
{
	Super::BeginDestroy();

	//移除流送的关卡
	if (StreamingPocketLevel)
	{
		StreamingPocketLevel->bShouldBlockOnUnload = false;
		StreamingPocketLevel->SetShouldBeLoaded(false);
		StreamingPocketLevel->OnLevelShown.RemoveAll(this);
		StreamingPocketLevel->OnLevelLoaded.RemoveAll(this);
		StreamingPocketLevel = nullptr;
	}
}

void UPocketLevelInstance::StreamIn()
{
	if (StreamingPocketLevel)
	{
		StreamingPocketLevel->SetShouldBeVisible(true);
		StreamingPocketLevel->SetShouldBeLoaded(true);
	}
}

void UPocketLevelInstance::StreamOut()
{
	if (StreamingPocketLevel)
	{
		StreamingPocketLevel->SetShouldBeVisible(false);
		StreamingPocketLevel->SetShouldBeLoaded(false);
	}
}

FDelegateHandle UPocketLevelInstance::AddReadyCallback(FPocketLevelInstanceEvent::FDelegate Callback)
{
	if (StreamingPocketLevel->GetLevelStreamingState() == ELevelStreamingState::LoadedVisible)
		Callback.ExecuteIfBound(this);

	return OnReadyEvent.Add(Callback);
}

void UPocketLevelInstance::RemoveReadyCallback(FDelegateHandle CallbackToRemove)
{
	OnReadyEvent.Remove(CallbackToRemove);
}

bool UPocketLevelInstance::Initialize(ULocalPlayer* InLocalPlayer, UPocketLevelAsset* InPocketLevel, const FVector& InSpawnPoint)
{
	LocalPlayer = InLocalPlayer;
	World = LocalPlayer->GetWorld();
	PocketLevel = InPocketLevel;
	Bounds = FBoxSphereBounds(FSphere(InSpawnPoint, PocketLevel->Bounds.GetAbsMax()));

	if (ensure(StreamingPocketLevel == nullptr))
	{
		if (ensure(!PocketLevel->Level.IsNull()))
		{
			bool bSuccess = false;
			StreamingPocketLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(LocalPlayer, PocketLevel->Level, Bounds.Origin, FRotator::ZeroRotator, bSuccess);

			if (ensure(bSuccess && StreamingPocketLevel))
			{
				StreamingPocketLevel->OnLevelLoaded.AddUniqueDynamic(this, &ThisClass::HandlePocketLevelLoaded);
				StreamingPocketLevel->OnLevelShown.AddUniqueDynamic(this, &ThisClass::HandlePocketLevelShown);
			}

			return bSuccess;
		}
	}

	return false;
}

void UPocketLevelInstance::HandlePocketLevelLoaded()
{
	if (StreamingPocketLevel)
	{
		//在级别设置中进行所有设置，以便在客户端上进行设置，然后我们处理
		//所有内容都是本地生成的，而不是bExchangedRoles=true，在那里生成
		//在客户端上，但期望是服务器命令执行，服务器将执行
		//稍后再告诉我们。
		if (ULevel* LoadedLevel = StreamingPocketLevel->GetLoadedLevel())
		{
			LoadedLevel->bClientOnlyVisible = true;

			for (AActor* Actor : LoadedLevel->Actors)
			{
				if (Actor) Actor->bExchangedRoles = true; //HACK，删除时bClientOnlyVisible是我们所需要的。
			}

			// TODO: 不要将所有权放在共享口袋空间上。
			if (LocalPlayer)
				if (APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld()))
					for (AActor* Actor : LoadedLevel->Actors)
						if (Actor) Actor->SetOwner(PC);
		}
	}
}

void UPocketLevelInstance::HandlePocketLevelShown()
{
	OnReadyEvent.Broadcast(this);
}
