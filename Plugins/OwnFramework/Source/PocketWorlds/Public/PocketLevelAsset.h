// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "PocketLevelAsset.generated.h"

#define UE_API POCKETWORLDS_API

/*用于PocketLevel的关卡信息资产*/
UCLASS(MinimalAPI)
class UPocketLevelAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// 将要流送,打开,加载的关卡软引用
	UPROPERTY(EditAnywhere, Category = "Streaming")
	TSoftObjectPtr<UWorld> Level;

	// 关卡的边界大小
	UPROPERTY(EditAnywhere, Category = "Streaming")
	FVector Bounds;
};

#undef UE_API
