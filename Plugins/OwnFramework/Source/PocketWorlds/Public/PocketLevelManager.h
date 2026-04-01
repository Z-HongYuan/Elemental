// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "PocketLevelManager.generated.h"

#define UE_API POCKETWORLDS_API

class UPocketLevelAsset;
class UPocketLevelInstance;
/**
 * 
 */
UCLASS(MinimalAPI)
class UPocketLevelManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/*获取或创建口袋关卡/小型关卡*/
	UE_API UPocketLevelInstance* GetOrCreatePocketLevelFor(ULocalPlayer* LocalPlayer, UPocketLevelAsset* PocketLevel, const FVector& DesiredSpawnPoint);

private:
	UPROPERTY()
	TArray<TObjectPtr<UPocketLevelInstance>> PocketInstances;
};
#undef UE_API
