// Copyright © 2026 鸿源z. All Rights Reserved.


#include "Widgets/ModularUIManagerSubsystem.h"

#include "Widgets/ModularUIPolicy.h"

UModularUIManagerSubsystem::UModularUIManagerSubsystem()
{
}

void UModularUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!CurrentPolicy && !DefaultUIPolicyClass.IsNull())
	{
		const TSubclassOf<UModularUIPolicy> PolicyClass = DefaultUIPolicyClass.LoadSynchronous();
		SwitchToPolicy(NewObject<UModularUIPolicy>(this, PolicyClass));
	}
}

void UModularUIManagerSubsystem::Deinitialize()
{
	SwitchToPolicy(nullptr);

	Super::Deinitialize();
}

bool UModularUIManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		//只有在其他地方没有定义覆盖实现的情况下才创建实例
		return ChildClasses.Num() == 0;
	}

	return false;
}

void UModularUIManagerSubsystem::NotifyPlayerAdded(UModularLocalPlayer* LocalPlayer)
{
	if (ensure(LocalPlayer) && CurrentPolicy)
		CurrentPolicy->NotifyPlayerAdded(LocalPlayer);
}

void UModularUIManagerSubsystem::NotifyPlayerRemoved(UModularLocalPlayer* LocalPlayer)
{
	if (LocalPlayer && CurrentPolicy)
		CurrentPolicy->NotifyPlayerRemoved(LocalPlayer);
}

void UModularUIManagerSubsystem::NotifyPlayerDestroyed(UModularLocalPlayer* LocalPlayer)
{
	if (LocalPlayer && CurrentPolicy)
		CurrentPolicy->NotifyPlayerDestroyed(LocalPlayer);
}

void UModularUIManagerSubsystem::SwitchToPolicy(UModularUIPolicy* InPolicy)
{
	if (CurrentPolicy != InPolicy)
		CurrentPolicy = InPolicy;
}
