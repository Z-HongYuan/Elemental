// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularPawn.h"
#include "Components/GameFrameworkComponentManager.h"

void AModularPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	//注册Pawn为接收者
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AModularPawn::BeginPlay()
{
	//发送GameActorReady事件
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

void AModularPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//移除Pawn为接收者
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}
