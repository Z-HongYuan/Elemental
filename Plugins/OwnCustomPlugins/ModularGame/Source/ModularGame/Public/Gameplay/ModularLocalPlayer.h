// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "ModularLocalPlayer.generated.h"

#define UE_API MODULARGAME_API

class UPrimaryLayoutWidget;
/**
 * 
 */
UCLASS(MinimalAPI, Config=Engine, Transient)
class UModularLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	UE_API UModularLocalPlayer();

	/** 当本地玩家被分配了玩家控制器时调用 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerControllerSetDelegate, UModularLocalPlayer* LocalPlayer, APlayerController* PlayerController);
	FPlayerControllerSetDelegate OnPlayerControllerSet;

	/** 当本地玩家被分配玩家状态时调用 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerStateSetDelegate, UModularLocalPlayer* LocalPlayer, APlayerState* PlayerState);
	FPlayerStateSetDelegate OnPlayerStateSet;

	/** 当本地玩家被分配到玩家Pawn时调用 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerPawnSetDelegate, UModularLocalPlayer* LocalPlayer, APawn* Pawn);
	FPlayerPawnSetDelegate OnPlayerPawnSet;

	UE_API FDelegateHandle CallAndRegister_OnPlayerControllerSet(FPlayerControllerSetDelegate::FDelegate Delegate);
	UE_API FDelegateHandle CallAndRegister_OnPlayerStateSet(FPlayerStateSetDelegate::FDelegate Delegate);
	UE_API FDelegateHandle CallAndRegister_OnPlayerPawnSet(FPlayerPawnSetDelegate::FDelegate Delegate);

public:
	UE_API virtual bool GetProjectionData(FViewport* Viewport, FSceneViewProjectionData& ProjectionData, int32 StereoViewIndex) const override;

	bool IsPlayerViewEnabled() const { return bIsPlayerViewEnabled; }
	void SetIsPlayerViewEnabled(bool bInIsPlayerViewEnabled) { bIsPlayerViewEnabled = bInIsPlayerViewEnabled; }

	UE_API UPrimaryLayoutWidget* GetRootUILayout() const;

private:
	bool bIsPlayerViewEnabled = true;
};
#undef UE_API
