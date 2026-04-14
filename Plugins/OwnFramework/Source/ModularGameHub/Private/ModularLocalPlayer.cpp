// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularGameHub/Public/ModularLocalPlayer.h"

#include "System/ModularUIManager.h"
#include "System/ModularUIPolicy.h"

FDelegateHandle UModularLocalPlayer::CallAndRegister_OnPlayerControllerSet(FPlayerControllerSetDelegate::FDelegate Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());

	//如果PC有效的话,马上调用传入的委托函数
	if (PC) Delegate.Execute(this, PC);

	return GetOnPlayerControllerSetDelegate().Add(Delegate);
}

FDelegateHandle UModularLocalPlayer::CallAndRegister_OnPlayerStateSet(FPlayerStateSetDelegate::FDelegate Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());
	APlayerState* PlayerState = PC ? PC->PlayerState : nullptr;

	//如果PS有效的话,马上调用传入的委托函数
	if (PlayerState) Delegate.Execute(this, PlayerState);

	return GetOnPlayerStateSetDelegate().Add(Delegate);
}

FDelegateHandle UModularLocalPlayer::CallAndRegister_OnPlayerPawnSet(FPlayerPawnSetDelegate::FDelegate Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;

	//如果Pawn有效的话,马上调用传入的委托函数
	if (Pawn) Delegate.Execute(this, Pawn);

	return GetOnPlayerPawnSetDelegate().Add(Delegate);
}

bool UModularLocalPlayer::GetProjectionData(FViewport* Viewport, FSceneViewProjectionData& ProjectionData, int32 StereoViewIndex) const
{
	//如果禁用视口的话,直接返回 false,不会处理视口数据
	if (!IsPlayerViewEnabled()) return false;

	return Super::GetProjectionData(Viewport, ProjectionData, StereoViewIndex);
}

UModularRootLayoutWidget* UModularLocalPlayer::GetRootUILayout() const
{
	UModularUIManager* UIManager = GetGameInstance()->GetSubsystem<UModularUIManager>();
	if (!UIManager) return nullptr;

	const UModularUIPolicy* Policy = UIManager->GetCurrentUIPolicy();
	if (!Policy) return nullptr;

	return Policy->GetModularRootLayoutWidget(this);
}
