// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularPlayerController.h"

#include "Components/ControllerComponent.h"
#include "Components/GameFrameworkComponentManager.h"

void AModularPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	//注册玩家控制器为接收者
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AModularPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//移除玩家控制器为接收者
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

void AModularPlayerController::ReceivedPlayer()
{
	// 玩家控制器总是会被分配一个玩家，在那之前不能做太多事情
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::ReceivedPlayer();

	// 调用所有模块化组件的ReceivedPlayer
	TArray<UControllerComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UControllerComponent* Component : ModularComponents)
	{
		Component->ReceivedPlayer();
	}
}

void AModularPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// 调用所有模块化组件的PlayerTick
	TArray<UControllerComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UControllerComponent* Component : ModularComponents)
	{
		Component->PlayerTick(DeltaTime);
	}
}
