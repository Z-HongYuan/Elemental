// Copyright © 2026 鸿源z. All Rights Reserved.


#include "System/ModularUIPolicy.h"
#include "Kismet/GameplayStatics.h"
#include "Logs/ModularUILogs.h"
#include "System/ModularUIManager.h"
#include "Widgets/ModularRootLayoutWidget.h"

UModularUIPolicy* UModularUIPolicy::GetModularUIPolicy(const UObject* WorldContextObject)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance) return nullptr;

	UModularUIManager* ModularUIManager = GameInstance->GetSubsystem<UModularUIManager>();
	if (!ModularUIManager) return nullptr;

	//获取保存在子系统中的指针
	return ModularUIManager->GetCurrentUIPolicy();
}

UWorld* UModularUIPolicy::GetWorld() const
{
	return GetOwningUIManager()->GetGameInstance()->GetWorld();
}

UModularUIManager* UModularUIPolicy::GetOwningUIManager() const
{
	return CastChecked<UModularUIManager>(GetOuter());
}

UModularRootLayoutWidget* UModularUIPolicy::GetModularRootLayoutWidget(const ULocalPlayer* LocalPlayer) const
{
	const FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer);
	return LayoutInfo ? LayoutInfo->RootLayoutWidget : nullptr;
}

void UModularUIPolicy::RequestPrimaryControl(UModularRootLayoutWidget* Layout)
{
	//仅在单人单屏模式下,且当前布局处于休眠状态时,将当前布局激活,适用场景为单屏多本地用户
	if (LocalMultiplayerViewMode == ELocalMultiplayerViewMode::SingleToggle && Layout->IsDormant())
	{
		for (const FRootViewportLayoutInfo& LayoutInfo : RootViewportLayouts)
		{
			UModularRootLayoutWidget* RootLayout = LayoutInfo.RootLayoutWidget;
			if (RootLayout && !RootLayout->IsDormant())
			{
				RootLayout->SetIsDormant(true);
				break;
			}
		}
		Layout->SetIsDormant(false);
	}
}

void UModularUIPolicy::AddLayoutToViewport(ULocalPlayer* LocalPlayer, UModularRootLayoutWidget* Layout)
{
	UE_LOG(LogModularUI, Log, TEXT("[%s] is adding player [%s]'s root layout [%s] to the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

	//设置Root根控件所关联的本地玩家
	Layout->SetPlayerContext(FLocalPlayerContext(LocalPlayer));
	//添加到玩家视口
	Layout->AddToPlayerScreen(1000);

	OnRootLayoutAddedToViewport(LocalPlayer, Layout);
}

void UModularUIPolicy::RemoveLayoutFromViewport(ULocalPlayer* LocalPlayer, UModularRootLayoutWidget* Layout)
{
	TWeakPtr<SWidget> LayoutSlateWidget = Layout->GetCachedWidget();
	if (LayoutSlateWidget.IsValid())
	{
		UE_LOG(LogModularUI, Log, TEXT("[%s] is removing player [%s]'s root layout [%s] from the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

		Layout->RemoveFromParent();
		if (LayoutSlateWidget.IsValid())
		{
			UE_LOG(LogModularUI, Warning, TEXT("Player [%s]'s root layout [%s] has been removed from the viewport, but other references to its underlying Slate widget still exist. Noting in case we leak it."),
			       *GetNameSafe(LocalPlayer),
			       *GetNameSafe(Layout));
		}

		OnRootLayoutRemovedFromViewport(LocalPlayer, Layout);
	}
}

void UModularUIPolicy::OnRootLayoutAddedToViewport(ULocalPlayer* LocalPlayer, UModularRootLayoutWidget* Layout)
{
#if WITH_EDITOR
	if (GIsEditor && LocalPlayer->IsPrimaryPlayer())
	{
		// 自动将键盘/鼠标焦点设置到游戏视口,不需要点击
		FSlateApplication::Get().SetUserFocusToGameViewport(0);
	}
#endif
}

void UModularUIPolicy::OnRootLayoutRemovedFromViewport(ULocalPlayer* LocalPlayer, UModularRootLayoutWidget* Layout)
{
}

void UModularUIPolicy::OnRootLayoutReleased(ULocalPlayer* LocalPlayer, UModularRootLayoutWidget* Layout)
{
}

void UModularUIPolicy::CreateLayoutWidgetAndAddToViewport(ULocalPlayer* LocalPlayer)
{
	APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld());
	if (!PlayerController) return;

	//同步加载,防止异步的时序问题
	TSubclassOf<UModularRootLayoutWidget> LayoutWidgetClass = GetLayoutWidgetClass();

	//检查类指针是否有效,且不为抽象类
	if (ensure(LayoutWidgetClass && !LayoutWidgetClass->HasAnyClassFlags(CLASS_Abstract)))
	{
		UModularRootLayoutWidget* NewLayoutObject = CreateWidget<UModularRootLayoutWidget>(PlayerController, LayoutWidgetClass);
		RootViewportLayouts.Emplace(LocalPlayer, NewLayoutObject, true);

		AddLayoutToViewport(LocalPlayer, NewLayoutObject);
	}
}

void UModularUIPolicy::NotifyPlayerAdded(ULocalPlayer* LocalPlayer)
{
	//绑定在玩家控制器更改时,会自动移除和添加一遍视口控件
	//刷新一遍视口吗,我也不知道为什么要这么做?
	//只要时序正确就行,Lyra中是在 ReceivedPlayer() 时机调用的,通过PC侧传递的事件链
	LocalPlayer->OnPlayerControllerChanged().AddWeakLambda(this, [this](const APlayerController* NewPC)
	{
		ULocalPlayer* NewLocalPlayer = NewPC->GetLocalPlayer();
		NotifyPlayerRemoved(NewLocalPlayer);

		if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(NewLocalPlayer))
		{
			AddLayoutToViewport(NewLocalPlayer, LayoutInfo->RootLayoutWidget);
			LayoutInfo->bAddedToViewport = true;
		}
		else
		{
			CreateLayoutWidgetAndAddToViewport(NewLocalPlayer);
		}
	});

	//判断是新玩家还是旧玩家,是否在数组中存在过
	if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		AddLayoutToViewport(LocalPlayer, LayoutInfo->RootLayoutWidget);
		LayoutInfo->bAddedToViewport = true;
	}
	else
	{
		CreateLayoutWidgetAndAddToViewport(LocalPlayer);
	}
}

void UModularUIPolicy::NotifyPlayerRemoved(ULocalPlayer* LocalPlayer)
{
	FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer);
	if (!LayoutInfo) return;

	//仅移除视口,不移除缓存
	RemoveLayoutFromViewport(LocalPlayer, LayoutInfo->RootLayoutWidget);
	LayoutInfo->bAddedToViewport = false;

	if (LocalMultiplayerViewMode == ELocalMultiplayerViewMode::SingleToggle && !LocalPlayer->IsPrimaryPlayer())
	{
		UModularRootLayoutWidget* RootLayout = LayoutInfo->RootLayoutWidget;
		if (RootLayout && !RootLayout->IsDormant())
		{
			// 移除次玩家的控件,并且激活主玩家
			RootLayout->SetIsDormant(true);
			for (const FRootViewportLayoutInfo& RootLayoutInfo : RootViewportLayouts)
			{
				if (RootLayoutInfo.LocalPlayer->IsPrimaryPlayer())
				{
					if (UModularRootLayoutWidget* PrimaryRootLayout = RootLayoutInfo.RootLayoutWidget)
					{
						PrimaryRootLayout->SetIsDormant(false);
					}
				}
			}
		}
	}
}

void UModularUIPolicy::NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer)
{
	NotifyPlayerRemoved(LocalPlayer);
	LocalPlayer->OnPlayerControllerChanged().RemoveAll(this);
	const int32 LayoutInfoIdx = RootViewportLayouts.IndexOfByKey(LocalPlayer);
	if (LayoutInfoIdx != INDEX_NONE)
	{
		//移除视口控件,并且移除缓存
		UModularRootLayoutWidget* Layout = RootViewportLayouts[LayoutInfoIdx].RootLayoutWidget;
		RootViewportLayouts.RemoveAt(LayoutInfoIdx);

		RemoveLayoutFromViewport(LocalPlayer, Layout);

		OnRootLayoutReleased(LocalPlayer, Layout);
	}
}
