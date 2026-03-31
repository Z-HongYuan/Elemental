// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularWithPlayerController.h"
#include "Components/ControllerComponent.h"
#include "Components/GameFrameworkComponentManager.h"

void AModularWithPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AModularWithPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	Super::EndPlay(EndPlayReason);
}

void AModularWithPlayerController::ReceivedPlayer()
{
	// 玩家控制器总是会分配一个玩家，在那之前不能做太多事情?未知语言
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::ReceivedPlayer();

	TArray<UControllerComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UControllerComponent* Component : ModularComponents)
		Component->ReceivedPlayer();
}

void AModularWithPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	TArray<UControllerComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UControllerComponent* Component : ModularComponents)
		Component->PlayerTick(DeltaTime);
}
