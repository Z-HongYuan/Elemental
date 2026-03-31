// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "PocketLevelSubsystem.generated.h"

#define UE_API POCKETWORLDS_API
class UPocketLevel;
class UPocketLevelInstance;
/**
 * 
 */
UCLASS(MinimalAPI)
class UPocketLevelSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UE_API UPocketLevelInstance* GetOrCreatePocketLevelFor(ULocalPlayer* LocalPlayer, UPocketLevel* PocketLevel, FVector DesiredSpawnPoint);

private:
	UPROPERTY()
	TArray<TObjectPtr<UPocketLevelInstance>> PocketInstances;
};
#undef UE_API
