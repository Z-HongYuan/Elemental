// Copyright © 2026 鸿源z. All Rights Reserved.


#include "ModularPlayerState.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Components/PlayerStateComponent.h"

void AModularPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	//注册玩家状态为接收者
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AModularPlayerState::BeginPlay()
{
	//发送GameActorReady事件
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

void AModularPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//移除玩家状态为接收者
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

void AModularPlayerState::Reset()
{
	Super::Reset();

	//将添加的所有模块化的组件进行重置
	TArray<UPlayerStateComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UPlayerStateComponent* Component : ModularComponents)
	{
		Component->Reset();
	}
}

void AModularPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	//将所有组件的属性复制给目标组件
	TInlineComponentArray<UPlayerStateComponent*> PlayerStateComponents; //性能更好的内联数组
	GetComponents(PlayerStateComponents);
	for (UPlayerStateComponent* SourcePSComp : PlayerStateComponents)
	{
		if (UPlayerStateComponent* TargetComp = Cast<UPlayerStateComponent>(
			static_cast<UObject*>(FindObjectWithOuter(PlayerState, SourcePSComp->GetClass(), SourcePSComp->GetFName()))))
		{
			SourcePSComp->CopyProperties(TargetComp);
		}
	}
}
