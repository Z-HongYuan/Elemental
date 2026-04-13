// Copyright © 2026 鸿源z. All Rights Reserved.


#include "System/ModularUIManager.h"
#include "System/ModularUIPolicy.h"

void UModularUIManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 当 策略指针无效,且有策略类 时,创建一个新的策略并且切换到到它
	//填充 CurrentPolicy 指针
	if (!CurrentPolicy && !DefaultUIPolicyClass.IsNull())
	{
		TSubclassOf<UModularUIPolicy> PolicyClass = DefaultUIPolicyClass.LoadSynchronous();
		UModularUIPolicy* Policy = NewObject<UModularUIPolicy>(this, PolicyClass);
		SwitchToPolicy(Policy);
	}
}

void UModularUIManager::Deinitialize()
{
	Super::Deinitialize();

	//清空 CurrentPolicy 指针
	SwitchToPolicy(nullptr);
}

bool UModularUIManager::ShouldCreateSubsystem(UObject* Outer) const
{
	//保证继承链中单例,且排除专用服务器
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		// 只有在其他地方没有定义覆盖实现的情况下才创建实例
		return ChildClasses.Num() == 0;
	}

	return false;
}

void UModularUIManager::NotifyPlayerAdded(ULocalPlayer* LocalPlayer)
{
	if (ensure(LocalPlayer) && CurrentPolicy)
		CurrentPolicy->NotifyPlayerAdded(LocalPlayer);
}

void UModularUIManager::NotifyPlayerRemoved(ULocalPlayer* LocalPlayer)
{
	if (ensure(LocalPlayer) && CurrentPolicy)
		CurrentPolicy->NotifyPlayerRemoved(LocalPlayer);
}

void UModularUIManager::NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer)
{
	if (ensure(LocalPlayer) && CurrentPolicy)
		CurrentPolicy->NotifyPlayerDestroyed(LocalPlayer);
}

void UModularUIManager::SwitchToPolicy(UModularUIPolicy* InPolicy)
{
	// 仅当不一致的时候才切换,忽略相同的情况
	if (CurrentPolicy != InPolicy) CurrentPolicy = InPolicy;
}
