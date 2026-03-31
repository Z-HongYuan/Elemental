// Copyright © 2026 鸿源z. All Rights Reserved.


#include "Widgets/PrimaryLayoutWidget.h"

#include "ModularGame.h"
#include "Gameplay/ModularLocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/ModularUIManagerSubsystem.h"
#include "Widgets/ModularUIPolicy.h"

UPrimaryLayoutWidget* UPrimaryLayoutWidget::GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	APlayerController* PlayerController = GameInstance->GetPrimaryPlayerController(false);
	return GetPrimaryGameLayout(PlayerController);
}

UPrimaryLayoutWidget* UPrimaryLayoutWidget::GetPrimaryGameLayout(APlayerController* PlayerController)
{
	return PlayerController ? GetPrimaryGameLayout(Cast<ULocalPlayer>(PlayerController->Player)) : nullptr;
}

UPrimaryLayoutWidget* UPrimaryLayoutWidget::GetPrimaryGameLayout(ULocalPlayer* LocalPlayer)
{
	if (!LocalPlayer) return nullptr;

	const UModularLocalPlayer* CommonLocalPlayer = CastChecked<UModularLocalPlayer>(LocalPlayer);

	if (const UGameInstance* GameInstance = CommonLocalPlayer->GetGameInstance())
		if (UModularUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UModularUIManagerSubsystem>())
			if (const UModularUIPolicy* Policy = UIManager->GetCurrentUIPolicy())
				if (UPrimaryLayoutWidget* RootLayout = Policy->GetRootLayout(CommonLocalPlayer))
					return RootLayout;

	return nullptr;
}

UPrimaryLayoutWidget::UPrimaryLayoutWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UPrimaryLayoutWidget::SetIsDormant(bool InDormant)
{
	if (bIsDormant != InDormant)
	{
		const ULocalPlayer* LP = GetOwningLocalPlayer();
		const int32 PlayerId = LP ? LP->GetControllerId() : -1;
		const TCHAR* OldDormancyStr = bIsDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
		const TCHAR* NewDormancyStr = InDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
		const TCHAR* PrimaryPlayerStr = LP && LP->IsPrimaryPlayer() ? TEXT("[Primary]") : TEXT("[Non-Primary]");
		UE_LOG(LogModularGame, Display, TEXT("%s PrimaryGameLayout Dormancy changed for [%d] from [%s] to [%s]"), PrimaryPlayerStr, PlayerId, OldDormancyStr, NewDormancyStr);

		bIsDormant = InDormant;
		OnIsDormantChanged();
	}
}

void UPrimaryLayoutWidget::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	//不确定小部件在那一层,所有层都尝试移除
	for (const auto& LayerKVP : Layers)
		LayerKVP.Value->RemoveWidget(*ActivatableWidget);
}

UCommonActivatableWidgetContainerBase* UPrimaryLayoutWidget::GetLayerWidget(FGameplayTag LayerName)
{
	return Layers.FindRef(LayerName);
}

void UPrimaryLayoutWidget::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	if (!IsDesignTime())
	{
		LayerWidget->OnTransitioningChanged.AddUObject(this, &UPrimaryLayoutWidget::OnWidgetStackTransitioning);
		//TODO:考虑允许转换持续时间，我们目前将其设置为0，因为如果它不是0使用时，过渡效果会导致焦点无法正确过渡到新的小部件 游戏手柄总是。
		LayerWidget->SetTransitionDuration(0.0);

		Layers.Add(LayerTag, LayerWidget);
	}
}

void UPrimaryLayoutWidget::OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning)
{
	if (bIsTransitioning)
	{
		const FName SuspendToken = UModularUIExtensions::SuspendInputForPlayer(GetOwningLocalPlayer(), TEXT("GlobalStackTransion"));
		SuspendInputTokens.Add(SuspendToken);
	}
	else
	{
		if (ensure(SuspendInputTokens.Num() > 0))
		{
			const FName SuspendToken = SuspendInputTokens.Pop();
			UModularUIExtensions::ResumeInputForPlayer(GetOwningLocalPlayer(), SuspendToken);
		}
	}
}

void UPrimaryLayoutWidget::OnIsDormantChanged()
{
	//@TODO NDarnell确定如何处理休眠，过去我们将休眠视为关闭渲染的一种方式
	//and the view for the other local players when we force multiple players to use the player view of a single player.

	//if (UCommonLocalPlayer* LocalPlayer = GetOwningLocalPlayer<UCommonLocalPlayer>())
	//{
	//	// When the root layout is dormant, we don't want to render anything from the owner's view either
	//	LocalPlayer->SetIsPlayerViewEnabled(!bIsDormant);
	//}

	//SetVisibility(bIsDormant ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);

	//OnLayoutDormancyChanged().Broadcast(bIsDormant);
}
