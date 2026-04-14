// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "Engine/LocalPlayer.h"
#include "ModularLocalPlayer.generated.h"

#define UE_API MODULARGAMEHUB_API

class UModularRootLayoutWidget;
/**
 * 
 */
UCLASS(MinimalAPI, Config=Engine, Transient)
class UModularLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	/** 当本地玩家被分配了玩家控制器时调用 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerControllerSetDelegate, UModularLocalPlayer* LocalPlayer, APlayerController* PlayerController);
	FPlayerControllerSetDelegate& GetOnPlayerControllerSetDelegate() { return OnPlayerControllerSet; }
	UE_API FDelegateHandle CallAndRegister_OnPlayerControllerSet(FPlayerControllerSetDelegate::FDelegate Delegate);

	/** 当本地玩家被分配玩家状态时调用 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerStateSetDelegate, UModularLocalPlayer* LocalPlayer, APlayerState* PlayerState);
	FPlayerStateSetDelegate& GetOnPlayerStateSetDelegate() { return OnPlayerStateSet; }
	UE_API FDelegateHandle CallAndRegister_OnPlayerStateSet(FPlayerStateSetDelegate::FDelegate Delegate);

	/** 当本地玩家被分配到玩家Pawn时调用 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerPawnSetDelegate, UModularLocalPlayer* LocalPlayer, APawn* Pawn);
	FPlayerPawnSetDelegate& GetOnPlayerPawnSetDelegate() { return OnPlayerPawnSet; }
	UE_API FDelegateHandle CallAndRegister_OnPlayerPawnSet(FPlayerPawnSetDelegate::FDelegate Delegate);

	UE_API virtual bool GetProjectionData(FViewport* Viewport, FSceneViewProjectionData& ProjectionData, int32 StereoViewIndex) const override;

	/* bIsPlayerViewEnabled 的Getter和Setter*/
	bool IsPlayerViewEnabled() const { return bIsPlayerViewEnabled; }
	void SetIsPlayerViewEnabled(bool bInIsPlayerViewEnabled) { bIsPlayerViewEnabled = bInIsPlayerViewEnabled; }

	UE_API UModularRootLayoutWidget* GetRootUILayout() const;

protected:
	FPlayerControllerSetDelegate OnPlayerControllerSet;
	FPlayerStateSetDelegate OnPlayerStateSet;
	FPlayerPawnSetDelegate OnPlayerPawnSet;

	bool bIsPlayerViewEnabled = true;
};

#undef UE_API
