// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UPocketLevelInstance.generated.h"

#define UE_API POCKETWORLDS_API

class UPocketLevel;
class ULevelStreamingDynamic;
class UPocketLevelInstance;
DECLARE_MULTICAST_DELEGATE_OneParam(FPocketLevelInstanceEvent, UPocketLevelInstance*);

/**
 * 
 */
UCLASS(MinimalAPI, Within = PocketLevelSubsystem, BlueprintType)
class UPocketLevelInstance : public UObject
{
	GENERATED_BODY()

public:
	UE_API UPocketLevelInstance();

	UE_API virtual void BeginDestroy() override;

	UE_API void StreamIn();
	UE_API void StreamOut();

	UE_API FDelegateHandle AddReadyCallback(FPocketLevelInstanceEvent::FDelegate Callback);
	UE_API void RemoveReadyCallback(FDelegateHandle CallbackToRemove);

	virtual class UWorld* GetWorld() const override { return World; }

private:
	UE_API bool Initialize(ULocalPlayer* InLocalPlayer, UPocketLevel* InPocketLevel, FVector InSpawnPoint);

	UFUNCTION()
	UE_API void HandlePocketLevelLoaded();

	UFUNCTION()
	UE_API void HandlePocketLevelShown();

private:
	UPROPERTY()
	TObjectPtr<ULocalPlayer> LocalPlayer;

	UPROPERTY()
	TObjectPtr<UPocketLevel> PocketLevel;

	UPROPERTY()
	TObjectPtr<UWorld> World;

	UPROPERTY()
	TObjectPtr<ULevelStreamingDynamic> StreamingPocketLevel;

	FPocketLevelInstanceEvent OnReadyEvent;

	FBoxSphereBounds Bounds;

	friend class UPocketLevelSubsystem;
};
#undef UE_API
