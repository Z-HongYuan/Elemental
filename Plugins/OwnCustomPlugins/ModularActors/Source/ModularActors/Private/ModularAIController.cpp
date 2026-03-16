// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularAIController.h"
#include "Components/GameFrameworkComponentManager.h"

void AModularAIController::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	//注册AI控制器为接收者
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AModularAIController::BeginPlay()
{
	//发送GameActorReady事件
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

void AModularAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//移除AI控制器为接收者
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}
