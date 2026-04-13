// Copyright © 2026 鸿源z. All Rights Reserved.


#include "Widgets/ModularRootLayoutWidget.h"

#include "ModularUITags.h"
#include "Kismet/GameplayStatics.h"
#include "System/ModularUIManager.h"
#include "System/ModularUIPolicy.h"

DECLARE_LOG_CATEGORY_EXTERN(LogModularUI, Log, All); //注册Log分类
// DEFINE_LOG_CATEGORY(LogModularUI); //注册Log分类

UModularRootLayoutWidget* UModularRootLayoutWidget::GetModularRootLayoutWidget(const UObject* WorldContextObject)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	APlayerController* PlayerController = GameInstance->GetPrimaryPlayerController(false);
	return GetModularRootLayoutWidget(PlayerController);
}

UModularRootLayoutWidget* UModularRootLayoutWidget::GetModularRootLayoutWidget(const APlayerController* PlayerController)
{
	return PlayerController ? GetModularRootLayoutWidget(PlayerController->Player) : nullptr;
}

UModularRootLayoutWidget* UModularRootLayoutWidget::GetModularRootLayoutWidget(const ULocalPlayer* LocalPlayer)
{
	if (!LocalPlayer) return nullptr;

	const UGameInstance* GameInstance = LocalPlayer->GetGameInstance();
	if (!GameInstance) return nullptr;

	const UModularUIManager* ModularUIManager = GameInstance->GetSubsystem<UModularUIManager>();
	if (!ModularUIManager) return nullptr;

	const UModularUIPolicy* CurrentUIPolicy = ModularUIManager->GetCurrentUIPolicy();
	if (!CurrentUIPolicy) return nullptr;

	return CurrentUIPolicy->GetModularRootLayoutWidget(LocalPlayer);
}

void UModularRootLayoutWidget::SetIsDormant(bool InDormant)
{
	if (bIsDormant == InDormant) return;

	//打印Log,说明切换时的情况,一般来说使用到这里的场景都很特殊
	const ULocalPlayer* LP = GetOwningLocalPlayer();
	const int32 PlayerId = LP ? LP->GetControllerId() : -1;
	const TCHAR* OldDormancyStr = bIsDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
	const TCHAR* NewDormancyStr = InDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
	const TCHAR* PrimaryPlayerStr = LP && LP->IsPrimaryPlayer() ? TEXT("[Primary]") : TEXT("[Non-Primary]");
	UE_LOG(LogModularUI, Display, TEXT("%s PrimaryGameLayout Dormancy changed for [%d] from [%s] to [%s]"), PrimaryPlayerStr, PlayerId, OldDormancyStr, NewDormancyStr);

	bIsDormant = InDormant;
	OnIsDormantChanged();
}

void UModularRootLayoutWidget::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	//查找并移除所有容器中匹配的控件
	for (const auto& LayerKVP : Container_Layers)
	{
		LayerKVP.Value->RemoveWidget(*ActivatableWidget);
	}
}

UCommonActivatableWidgetContainerBase* UModularRootLayoutWidget::GetLayerContainer(const FGameplayTag LayerName)
{
	return Container_Layers.FindRef(LayerName);
}

void UModularRootLayoutWidget::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	if (IsDesignTime()) return;

	LayerWidget->OnTransitioningChanged.AddUObject(this, &UModularRootLayoutWidget::OnWidgetStackTransitioning);
	//TODO：考虑允许过渡时长，我们目前设为0，因为如果不是0，切换效果会导致焦点无法正确切换到新控件，尤其是使用手柄时。
	LayerWidget->SetTransitionDuration(0.0);

	Container_Layers.Add(LayerTag, LayerWidget);
}

void UModularRootLayoutWidget::OnIsDormantChanged()
{
	//@TODO 确定如何处理休眠，过去我们把休眠视为关闭渲染的一种方式 以及当我们强制多名玩家使用单人玩家的玩家视图时，其他本地玩家的视图。

	//if (UCommonLocalPlayer* LocalPlayer = GetOwningLocalPlayer<UCommonLocalPlayer>())
	//{
	//	// When the root layout is dormant, we don't want to render anything from the owner's view either
	//	LocalPlayer->SetIsPlayerViewEnabled(!bIsDormant);
	//}

	//SetVisibility(bIsDormant ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);

	//OnLayoutDormancyChanged().Broadcast(bIsDormant);
}

void UModularRootLayoutWidget::OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning)
{
	if (bIsTransitioning)
	{
		const FName SuspendToken = UModularUIHelperFunction::SuspendInputForPlayer(GetOwningLocalPlayer(), TEXT("GlobalStackTransion"));
		SuspendInputTokens.Add(SuspendToken);
	}
	else
	{
		if (ensure(SuspendInputTokens.Num() > 0))
		{
			const FName SuspendToken = SuspendInputTokens.Pop();
			UModularUIHelperFunction::ResumeInputForPlayer(GetOwningLocalPlayer(), SuspendToken);
		}
	}
}

void UModularRootLayoutWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	//默认注册了4个容器
	if (Container_Modal) RegisterLayer(ModularUITags::ModularUI_UIContainer_Modal, Container_Modal);
	if (Container_GameMenu) RegisterLayer(ModularUITags::ModularUI_UIContainer_GameMenu, Container_GameMenu);
	if (Container_GameHUD) RegisterLayer(ModularUITags::ModularUI_UIContainer_GameHUD, Container_GameHUD);
	if (Container_Frontend) RegisterLayer(ModularUITags::ModularUI_UIContainer_Frontend, Container_Frontend);
}
