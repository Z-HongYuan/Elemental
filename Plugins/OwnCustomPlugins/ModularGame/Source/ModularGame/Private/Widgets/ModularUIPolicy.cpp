// Copyright © 2026 鸿源z. All Rights Reserved.


#include "Widgets/ModularUIPolicy.h"

#include "ModularGame.h"
#include "Gameplay/ModularLocalPlayer.h"
#include "Widgets/ModularUIManagerSubsystem.h"
#include "Widgets/PrimaryLayoutWidget.h"

UModularUIPolicy::UModularUIPolicy(const FObjectInitializer& ObjectInitializer)
{
}

UModularUIPolicy* UModularUIPolicy::GetUModularUIPolicy(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		if (UGameInstance* GameInstance = World->GetGameInstance())
			if (UModularUIManagerSubsystem* UIManager = UGameInstance::GetSubsystem<UModularUIManagerSubsystem>(GameInstance))
				return UIManager->GetCurrentUIPolicy();

	return nullptr;
}

UWorld* UModularUIPolicy::GetWorld() const
{
	return GetOwningUIManager()->GetGameInstance()->GetWorld();
}

UModularUIManagerSubsystem* UModularUIPolicy::GetOwningUIManager() const
{
	return CastChecked<UModularUIManagerSubsystem>(GetOuter());
}

UPrimaryLayoutWidget* UModularUIPolicy::GetRootLayout(const UModularLocalPlayer* LocalPlayer) const
{
	const FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer);
	return LayoutInfo ? LayoutInfo->RootLayout : nullptr;
}

void UModularUIPolicy::RequestPrimaryControl(UPrimaryLayoutWidget* Layout)
{
	if (LocalMultiplayerInteractionMode == ELocalMultiplayerInteractionMode::SingleToggle && Layout->IsDormant())
	{
		for (const FRootViewportLayoutInfo& LayoutInfo : RootViewportLayouts)
		{
			UPrimaryLayoutWidget* RootLayout = LayoutInfo.RootLayout;
			if (RootLayout && !RootLayout->IsDormant())
			{
				RootLayout->SetIsDormant(true);
				break;
			}
		}
		Layout->SetIsDormant(false);
	}
}

void UModularUIPolicy::AddLayoutToViewport(UModularLocalPlayer* LocalPlayer, UPrimaryLayoutWidget* Layout)
{
	UE_LOG(LogModularGame, Log, TEXT("[%s] is adding player [%s]'s root layout [%s] to the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

	Layout->SetPlayerContext(FLocalPlayerContext(LocalPlayer));
	Layout->AddToPlayerScreen(1000);

	OnRootLayoutAddedToViewport(LocalPlayer, Layout);
}

void UModularUIPolicy::RemoveLayoutFromViewport(UModularLocalPlayer* LocalPlayer, UPrimaryLayoutWidget* Layout)
{
	TWeakPtr<SWidget> LayoutSlateWidget = Layout->GetCachedWidget();
	if (LayoutSlateWidget.IsValid())
	{
		UE_LOG(LogModularGame, Log, TEXT("[%s] is removing player [%s]'s root layout [%s] from the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

		Layout->RemoveFromParent();

		if (LayoutSlateWidget.IsValid())
			UE_LOG(LogModularGame, Log, TEXT("Player [%s]'s root layout [%s] has been removed from the viewport, but other references to its underlying Slate widget still exist. Noting in case we leak it."), *GetNameSafe(LocalPlayer),
		       *GetNameSafe(Layout));

		OnRootLayoutRemovedFromViewport(LocalPlayer, Layout);
	}
}

void UModularUIPolicy::OnRootLayoutAddedToViewport(UModularLocalPlayer* LocalPlayer, UPrimaryLayoutWidget* Layout)
{
#if WITH_EDITOR
	// 因此，我们的控制器将在PIE中工作，而无需在视口中单击
	if (GIsEditor && LocalPlayer->IsPrimaryPlayer())
		FSlateApplication::Get().SetUserFocusToGameViewport(0);

#endif
}

void UModularUIPolicy::OnRootLayoutRemovedFromViewport(UModularLocalPlayer* LocalPlayer, UPrimaryLayoutWidget* Layout)
{
}

void UModularUIPolicy::OnRootLayoutReleased(UModularLocalPlayer* LocalPlayer, UPrimaryLayoutWidget* Layout)
{
}

void UModularUIPolicy::CreateLayoutWidget(UModularLocalPlayer* LocalPlayer)
{
	if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
	{
		TSubclassOf<UPrimaryLayoutWidget> LayoutWidgetClass = GetLayoutWidgetClass(LocalPlayer);
		if (ensure(LayoutWidgetClass && !LayoutWidgetClass->HasAnyClassFlags(CLASS_Abstract)))
		{
			UPrimaryLayoutWidget* NewLayoutObject = CreateWidget<UPrimaryLayoutWidget>(PlayerController, LayoutWidgetClass);
			RootViewportLayouts.Emplace(LocalPlayer, NewLayoutObject, true);

			AddLayoutToViewport(LocalPlayer, NewLayoutObject);
		}
	}
}

TSubclassOf<UPrimaryLayoutWidget> UModularUIPolicy::GetLayoutWidgetClass(UModularLocalPlayer* LocalPlayer)
{
	//同步加载Class引用
	return LayoutClass.LoadSynchronous();
}

void UModularUIPolicy::NotifyPlayerAdded(UModularLocalPlayer* LocalPlayer)
{
	LocalPlayer->OnPlayerControllerSet.AddWeakLambda(this, [this](UModularLocalPlayer* LocalPlayer, APlayerController* PlayerController)
	{
		NotifyPlayerRemoved(LocalPlayer);

		if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
		{
			AddLayoutToViewport(LocalPlayer, LayoutInfo->RootLayout);
			LayoutInfo->bAddedToViewport = true;
		}
		else
		{
			CreateLayoutWidget(LocalPlayer);
		}
	});

	if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		AddLayoutToViewport(LocalPlayer, LayoutInfo->RootLayout);
		LayoutInfo->bAddedToViewport = true;
	}
	else
	{
		CreateLayoutWidget(LocalPlayer);
	}
}

void UModularUIPolicy::NotifyPlayerRemoved(UModularLocalPlayer* LocalPlayer)
{
	if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		RemoveLayoutFromViewport(LocalPlayer, LayoutInfo->RootLayout);
		LayoutInfo->bAddedToViewport = false;

		if (LocalMultiplayerInteractionMode == ELocalMultiplayerInteractionMode::SingleToggle && !LocalPlayer->IsPrimaryPlayer())
		{
			UPrimaryLayoutWidget* RootLayout = LayoutInfo->RootLayout;
			if (RootLayout && !RootLayout->IsDormant())
			{
				// 我们正在删除次要玩家的根，而它处于控制状态——将控制权转移回主要玩家的根
				RootLayout->SetIsDormant(true);
				for (const FRootViewportLayoutInfo& RootLayoutInfo : RootViewportLayouts)
				{
					if (RootLayoutInfo.LocalPlayer->IsPrimaryPlayer())
					{
						if (UPrimaryLayoutWidget* PrimaryRootLayout = RootLayoutInfo.RootLayout)
						{
							PrimaryRootLayout->SetIsDormant(false);
						}
					}
				}
			}
		}
	}
}

void UModularUIPolicy::NotifyPlayerDestroyed(UModularLocalPlayer* LocalPlayer)
{
	NotifyPlayerRemoved(LocalPlayer);
	LocalPlayer->OnPlayerControllerSet.RemoveAll(this);
	const int32 LayoutInfoIdx = RootViewportLayouts.IndexOfByKey(LocalPlayer);
	if (LayoutInfoIdx != INDEX_NONE)
	{
		UPrimaryLayoutWidget* Layout = RootViewportLayouts[LayoutInfoIdx].RootLayout;
		RootViewportLayouts.RemoveAt(LayoutInfoIdx);

		RemoveLayoutFromViewport(LocalPlayer, Layout);

		OnRootLayoutReleased(LocalPlayer, Layout);
	}
}
