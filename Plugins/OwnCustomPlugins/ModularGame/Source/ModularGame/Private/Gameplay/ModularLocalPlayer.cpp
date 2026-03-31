// Copyright © 2026 鸿源z. All Rights Reserved.


#include "Gameplay/ModularLocalPlayer.h"

#include "Widgets/ModularUIManagerSubsystem.h"
#include "Widgets/ModularUIPolicy.h"

UModularLocalPlayer::UModularLocalPlayer() : Super(FObjectInitializer::Get())
{
}

FDelegateHandle UModularLocalPlayer::CallAndRegister_OnPlayerControllerSet(FPlayerControllerSetDelegate::FDelegate Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());
	if (PC) Delegate.Execute(this, PC);
	return OnPlayerControllerSet.Add(Delegate);
}

FDelegateHandle UModularLocalPlayer::CallAndRegister_OnPlayerStateSet(FPlayerStateSetDelegate::FDelegate Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());
	APlayerState* PlayerState = PC ? PC->PlayerState : nullptr;
	if (PlayerState) Delegate.Execute(this, PlayerState);
	return OnPlayerStateSet.Add(Delegate);
}

FDelegateHandle UModularLocalPlayer::CallAndRegister_OnPlayerPawnSet(FPlayerPawnSetDelegate::FDelegate Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (Pawn) Delegate.Execute(this, Pawn);
	return OnPlayerPawnSet.Add(Delegate);
}

bool UModularLocalPlayer::GetProjectionData(FViewport* Viewport, FSceneViewProjectionData& ProjectionData, int32 StereoViewIndex) const
{
	if (!bIsPlayerViewEnabled) return false;
	return Super::GetProjectionData(Viewport, ProjectionData, StereoViewIndex);
}

UPrimaryLayoutWidget* UModularLocalPlayer::GetRootUILayout() const
{
	if (UModularUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UModularUIManagerSubsystem>())
		if (UModularUIPolicy* Policy = UIManager->GetCurrentUIPolicy())
			return Policy->GetRootLayout(this);

	return nullptr;
}
