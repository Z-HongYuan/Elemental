// Copyright © 2026 鸿源z. All Rights Reserved.


#include "System/ModularUIHelperFunction.h"

#include "CommonActivatableWidget.h"
#include "CommonInputSubsystem.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "System/ModularUIManager.h"
#include "System/ModularUIPolicy.h"
#include "Widgets/ModularRootLayoutWidget.h"

ECommonInputType UModularUIHelperFunction::GetOwningPlayerInputType(const UUserWidget* WidgetContextObject)
{
	if (!WidgetContextObject) return ECommonInputType::Count;

	if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
	{
		return InputSubsystem->GetCurrentInputType();
	}

	return ECommonInputType::Count;
}

bool UModularUIHelperFunction::IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject)
{
	if (!WidgetContextObject) return false;

	if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
	{
		return InputSubsystem->GetCurrentInputType() == ECommonInputType::Touch;
	}

	return false;
}

bool UModularUIHelperFunction::IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject)
{
	if (!WidgetContextObject) return false;

	if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
	{
		return InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
	}

	return false;
}

UCommonActivatableWidget* UModularUIHelperFunction::PushWidgetToLayer_ForPlayer(const ULocalPlayer* LocalPlayer, FGameplayTag LayerName, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!ensure(LocalPlayer) || !ensure(WidgetClass != nullptr)) return nullptr;

	const UGameInstance* GameInstance = LocalPlayer->GetGameInstance();
	if (!GameInstance) return nullptr;

	UModularUIManager* UIManager = GameInstance->GetSubsystem<UModularUIManager>();
	if (!UIManager) return nullptr;

	const UModularUIPolicy* Policy = UIManager->GetCurrentUIPolicy();
	if (!Policy) return nullptr;

	if (UModularRootLayoutWidget* RootLayout = Policy->GetModularRootLayoutWidget(LocalPlayer))
	{
		return RootLayout->PushWidgetToLayerStack(LayerName, WidgetClass);
	}

	return nullptr;
}

void UModularUIHelperFunction::PushWidgetToLayerAsync_ForPlayer(const ULocalPlayer* LocalPlayer, FGameplayTag LayerName, TSoftClassPtr<UCommonActivatableWidget> WidgetClass)
{
	if (!ensure(LocalPlayer) || !ensure(!WidgetClass.IsNull())) return;

	const UGameInstance* GameInstance = LocalPlayer->GetGameInstance();
	if (!GameInstance) return;

	UModularUIManager* UIManager = GameInstance->GetSubsystem<UModularUIManager>();
	if (!UIManager) return;

	const UModularUIPolicy* Policy = UIManager->GetCurrentUIPolicy();
	if (!Policy) return;

	if (UModularRootLayoutWidget* RootLayout = Policy->GetModularRootLayoutWidget(LocalPlayer))
	{
		constexpr bool bSuspendInputUntilComplete = true;
		RootLayout->PushWidgetToLayerStackAsync(LayerName, bSuspendInputUntilComplete, WidgetClass);
	}
}

void UModularUIHelperFunction::PopWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	if (!ActivatableWidget) return;

	const ULocalPlayer* LocalPlayer = ActivatableWidget->GetOwningLocalPlayer();
	if (!LocalPlayer) return;

	const UGameInstance* GameInstance = LocalPlayer->GetGameInstance();
	if (!GameInstance) return;

	UModularUIManager* UIManager = GameInstance->GetSubsystem<UModularUIManager>();
	if (!UIManager) return;

	const UModularUIPolicy* Policy = UIManager->GetCurrentUIPolicy();
	if (!Policy) return;

	if (UModularRootLayoutWidget* RootLayout = Policy->GetModularRootLayoutWidget(LocalPlayer))
	{
		RootLayout->FindAndRemoveWidgetFromLayer(ActivatableWidget);
	}
}

ULocalPlayer* UModularUIHelperFunction::GetLocalPlayerFromController(APlayerController* PlayerController)
{
	if (PlayerController) return PlayerController->GetLocalPlayer();

	return nullptr;
}

FName UModularUIHelperFunction::SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason)
{
	return SuspendInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendReason);
}

int32 UModularUIHelperFunction::InputSuspensions = 0; //初始化为0

FName UModularUIHelperFunction::SuspendInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendReason)
{
	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		InputSuspensions++;
		FName SuspendToken = SuspendReason;
		SuspendToken.SetNumber(InputSuspensions);

		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, true);

		return SuspendToken;
	}

	return NAME_None;
}

void UModularUIHelperFunction::ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken)
{
	ResumeInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendToken);
}

void UModularUIHelperFunction::ResumeInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendToken)
{
	if (SuspendToken == NAME_None) return;

	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, false);
	}
}
