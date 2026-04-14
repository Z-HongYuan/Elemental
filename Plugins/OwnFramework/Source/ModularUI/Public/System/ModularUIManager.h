// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "ModularUIManager.generated.h"

#define UE_API MODULARUI_API

class UModularUIPolicy;

/**
 * 只针对 UModularUIPolicy 的持有和管理
 */
UCLASS(MinimalAPI, Abstract, Config = Game)
class UModularUIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ~ UGameInstanceSubsystem Interface
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	// ~ UGameInstanceSubsystem Interface


	const UModularUIPolicy* GetCurrentUIPolicy() const { return CurrentPolicy; } //CurrentPolicy的Getter
	UModularUIPolicy* GetCurrentUIPolicy() { return CurrentPolicy; } //CurrentPolicy的Getter

	//传递到Policy中调用,其源头是在UGameInstance::NotifyPlayerAdded中，用于通知Policy有对应新玩家加入
	UE_API virtual void NotifyPlayerAdded(ULocalPlayer* LocalPlayer);

	//传递到Policy中调用,其源头是在UGameInstance::NotifyPlayerRemoved中，用于通知Policy有对应玩家离开
	UE_API virtual void NotifyPlayerRemoved(ULocalPlayer* LocalPlayer);

	//传递到Policy中调用,其源头是在UGameInstance::NotifyPlayerDestroyed中，用于通知Policy有对应玩家销毁
	UE_API virtual void NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer);

	UE_API void SwitchToPolicy(UModularUIPolicy* InPolicy);

private:
	UPROPERTY(Transient)
	TObjectPtr<UModularUIPolicy> CurrentPolicy = nullptr;

	//需要配置的默认Policy引用
	UPROPERTY(Config, EditAnywhere)
	TSoftClassPtr<UModularUIPolicy> DefaultUIPolicyClass;
};

#undef UE_API
