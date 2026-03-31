// Copyright © 2026 鸿源z. All Rights Reserved.


#include "Widgets/ModularUIExtensions.h"

#include "CommonActivatableWidget.h"
#include "CommonInputSubsystem.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/ModularLocalPlayer.h"
#include "Widgets/ModularUIManagerSubsystem.h"
#include "Widgets/ModularUIPolicy.h"
#include "Widgets/PrimaryLayoutWidget.h"

int32 UModularUIExtensions::InputSuspensions = 0;

ECommonInputType UModularUIExtensions::GetOwningPlayerInputType(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
			return InputSubsystem->GetCurrentInputType();

	return ECommonInputType::Count;
}

bool UModularUIExtensions::IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
			return InputSubsystem->GetCurrentInputType() == ECommonInputType::Touch;

	return false;
}

bool UModularUIExtensions::IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
			return InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;

	return false;
}

UCommonActivatableWidget* UModularUIExtensions::PushContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer, FGameplayTag LayerName, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!ensure(LocalPlayer) || !ensure(WidgetClass != nullptr)) return nullptr;

	if (UModularUIManagerSubsystem* UIManager = LocalPlayer->GetGameInstance()->GetSubsystem<UModularUIManagerSubsystem>())
		if (UModularUIPolicy* Policy = UIManager->GetCurrentUIPolicy())
			if (UPrimaryLayoutWidget* RootLayout = Policy->GetRootLayout(CastChecked<UModularLocalPlayer>(LocalPlayer)))
				return RootLayout->PushWidgetToLayerStack(LayerName, WidgetClass);

	return nullptr;
}

void UModularUIExtensions::PushStreamedContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer, FGameplayTag LayerName, TSoftClassPtr<UCommonActivatableWidget> WidgetClass)
{
	if (!ensure(LocalPlayer) || !ensure(!WidgetClass.IsNull())) return;


	if (UModularUIManagerSubsystem* UIManager = LocalPlayer->GetGameInstance()->GetSubsystem<UModularUIManagerSubsystem>())
	{
		if (UModularUIPolicy* Policy = UIManager->GetCurrentUIPolicy())
		{
			if (UPrimaryLayoutWidget* RootLayout = Policy->GetRootLayout(CastChecked<UModularLocalPlayer>(LocalPlayer)))
			{
				constexpr bool bSuspendInputUntilComplete = true;
				RootLayout->PushWidgetToLayerStackAsync(LayerName, bSuspendInputUntilComplete, WidgetClass);
			}
		}
	}
}

void UModularUIExtensions::PopContentFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	//检查是否无效
	if (!ActivatableWidget) return;

	if (const ULocalPlayer* LocalPlayer = ActivatableWidget->GetOwningLocalPlayer())
		if (const UModularUIManagerSubsystem* UIManager = LocalPlayer->GetGameInstance()->GetSubsystem<UModularUIManagerSubsystem>())
			if (const UModularUIPolicy* Policy = UIManager->GetCurrentUIPolicy())
				if (UPrimaryLayoutWidget* RootLayout = Policy->GetRootLayout(CastChecked<UModularLocalPlayer>(LocalPlayer)))
					RootLayout->FindAndRemoveWidgetFromLayer(ActivatableWidget);
}

ULocalPlayer* UModularUIExtensions::GetLocalPlayerFromController(APlayerController* PlayerController)
{
	if (PlayerController) return Cast<ULocalPlayer>(PlayerController->Player);
	return nullptr;
}

FName UModularUIExtensions::SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason)
{
	return SuspendInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendReason);
}

FName UModularUIExtensions::SuspendInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendReason)
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

void UModularUIExtensions::ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken)
{
	ResumeInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendToken);
}

void UModularUIExtensions::ResumeInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendToken)
{
	if (SuspendToken == NAME_None) return;
	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, false);
	}
}
