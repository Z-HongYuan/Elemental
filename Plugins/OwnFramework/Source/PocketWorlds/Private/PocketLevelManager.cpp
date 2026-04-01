// Copyright © 2026 鸿源z. All Rights Reserved.


#include "PocketWorlds/Public/PocketLevelManager.h"

#include "PocketWorlds/Public/PocketLevelAsset.h"
#include "PocketWorlds/Public/PocketLevelInstance.h"

UPocketLevelInstance* UPocketLevelManager::GetOrCreatePocketLevelFor(ULocalPlayer* LocalPlayer, UPocketLevelAsset* PocketLevel, const FVector& DesiredSpawnPoint)
{
	//如果资产无效直接返回
	if (PocketLevel == nullptr) return nullptr;

	float VerticalBoundsOffset = 0;
	for (UPocketLevelInstance* Instance : PocketInstances)
	{
		//检查是否已经存在,从 Owner 和资产进行匹配
		if (Instance->LocalPlayer == LocalPlayer && Instance->PocketLevel == PocketLevel)
			return Instance;

		//如果都匹配不上,就准备新创建一个关卡,这里叠加的所有关卡的垂直量,避免重叠
		VerticalBoundsOffset += Instance->PocketLevel->Bounds.Z;
	}

	const FVector SpawnPoint = DesiredSpawnPoint + FVector(0, 0, VerticalBoundsOffset);

	//创建并初始化关卡实例
	UPocketLevelInstance* NewInstance = NewObject<UPocketLevelInstance>(this);
	NewInstance->Initialize(LocalPlayer, PocketLevel, SpawnPoint);

	PocketInstances.Add(NewInstance);

	return NewInstance;
}
