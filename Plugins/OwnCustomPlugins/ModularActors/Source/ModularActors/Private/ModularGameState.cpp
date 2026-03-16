// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularGameState.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Components/GameStateComponent.h"


void AModularGameStateBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// 注册游戏状态为接收者
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AModularGameStateBase::BeginPlay()
{
	// 发送GameActorReady事件
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

void AModularGameStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 移除游戏状态为接收者
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

void AModularGameState::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// 注册游戏状态为接收者
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AModularGameState::BeginPlay()
{
	// 发送GameActorReady事件
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

void AModularGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 移除游戏状态为接收者
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

void AModularGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	//针对于模块化对象,在其附加的所有组件中调用组件对应的HandleMatchHasStarted
	TArray<UGameStateComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UGameStateComponent* Component : ModularComponents)
	{
		Component->HandleMatchHasStarted();
	}
}
