// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "PocketLevelInstance.generated.h"

#define UE_API POCKETWORLDS_API

class ULevelStreamingDynamic;
class UPocketLevelAsset;
class UPocketLevelInstance;

DECLARE_MULTICAST_DELEGATE_OneParam(FPocketLevelInstanceEvent, UPocketLevelInstance*);


/**/
UCLASS(MinimalAPI, Within = PocketLevelManager, BlueprintType)
class UPocketLevelInstance : public UObject
{
	GENERATED_BODY()

public:
	UE_API UPocketLevelInstance();

	UE_API virtual void BeginDestroy() override;

	UE_API void StreamIn(); //流送关卡
	UE_API void StreamOut(); //卸载关卡


	UE_API FDelegateHandle AddReadyCallback(FPocketLevelInstanceEvent::FDelegate Callback); //添加关卡准备就绪的回调
	UE_API void RemoveReadyCallback(FDelegateHandle CallbackToRemove); //移除关卡准备就绪的回调

	virtual UWorld* GetWorld() const override { return World; }

private:
	//使用关卡资产初始化关卡
	UE_API bool Initialize(ULocalPlayer* InLocalPlayer, UPocketLevelAsset* InPocketLevel, const FVector& InSpawnPoint);

	UFUNCTION()
	UE_API void HandlePocketLevelLoaded(); //关卡加载完成

	UFUNCTION()
	UE_API void HandlePocketLevelShown(); //关卡显示完成

	/*私有变量*/

	UPROPERTY()
	TObjectPtr<ULocalPlayer> LocalPlayer;

	UPROPERTY()
	TObjectPtr<UPocketLevelAsset> PocketLevel;

	UPROPERTY()
	TObjectPtr<UWorld> World;

	UPROPERTY()
	TObjectPtr<ULevelStreamingDynamic> StreamingPocketLevel;

	FPocketLevelInstanceEvent OnReadyEvent;

	FBoxSphereBounds Bounds;

	friend class UPocketLevelManager;
};
#undef UE_API
