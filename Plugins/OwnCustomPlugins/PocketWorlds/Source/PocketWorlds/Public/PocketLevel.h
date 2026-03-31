// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PocketLevel.generated.h"

#define UE_API POCKETWORLDS_API
/**
 * 
 */
UCLASS(MinimalAPI)
class UPocketLevel : public UDataAsset
{
	GENERATED_BODY()

public:
	UE_API UPocketLevel();

public:
	// The level that will be streamed in for this pocket level.
	UPROPERTY(EditAnywhere, Category = "Streaming")
	TSoftObjectPtr<UWorld> Level;

	// The bounds of the pocket level so that we can create multiple instances without overlapping each other.
	UPROPERTY(EditAnywhere, Category = "Streaming")
	FVector Bounds;
};
#undef UE_API
