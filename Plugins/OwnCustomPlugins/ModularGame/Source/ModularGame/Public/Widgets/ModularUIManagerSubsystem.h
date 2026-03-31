// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ModularUIManagerSubsystem.generated.h"

#define UE_API MODULARGAME_API

class UModularLocalPlayer;
class UModularUIPolicy;
/**
 * 管理模块化UI的子系统
 */
UCLASS(MinimalAPI, Abstract, config = Game)
class UModularUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UE_API UModularUIManagerSubsystem();

	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	const UModularUIPolicy* GetCurrentUIPolicy() const { return CurrentPolicy; }
	UModularUIPolicy* GetCurrentUIPolicy() { return CurrentPolicy; }

	UE_API virtual void NotifyPlayerAdded(UModularLocalPlayer* LocalPlayer);
	UE_API virtual void NotifyPlayerRemoved(UModularLocalPlayer* LocalPlayer);
	UE_API virtual void NotifyPlayerDestroyed(UModularLocalPlayer* LocalPlayer);

protected:
	UE_API void SwitchToPolicy(UModularUIPolicy* InPolicy);

private:
	UPROPERTY(Transient)
	TObjectPtr<UModularUIPolicy> CurrentPolicy = nullptr;

	UPROPERTY(config, EditAnywhere)
	TSoftClassPtr<UModularUIPolicy> DefaultUIPolicyClass;
};
#undef UE_API
