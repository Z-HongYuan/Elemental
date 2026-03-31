// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ModularUIPolicy.generated.h"

#define UE_API MODULARGAME_API

class UModularLocalPlayer;
class UModularUIManagerSubsystem;
class UPrimaryLayoutWidget;

/*用于区分视口模式的枚举*/
UENUM()
enum class ELocalMultiplayerInteractionMode : uint8
{
	// 仅适用于主要玩家的全屏视口，而不管其他玩家是否存在,一个人一个视口
	PrimaryOnly,

	// 一个玩家的全屏视口，但玩家可以交换对谁显示谁处于休眠状态的控制权,一个人两个视口
	SingleToggle,

	// 同时为两名玩家显示的视口,多个人多个视口
	Simultaneous
};

/*根控件使用的信息结构体,其中包括了 LocalPlayer,RootLayout,bAddedToViewport*/
USTRUCT()
struct FRootViewportLayoutInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UPrimaryLayoutWidget> RootLayout = nullptr;

	UPROPERTY(Transient)
	bool bAddedToViewport = false;

	FRootViewportLayoutInfo()
	{
	}

	FRootViewportLayoutInfo(ULocalPlayer* InLocalPlayer, UPrimaryLayoutWidget* InRootLayout, bool bIsInViewport)
		: LocalPlayer(InLocalPlayer)
		  , RootLayout(InRootLayout)
		  , bAddedToViewport(bIsInViewport)
	{
	}

	bool operator==(const ULocalPlayer* OtherLocalPlayer) const { return LocalPlayer == OtherLocalPlayer; }
};


/**
 * UI的布局策略类
 * 需要UIManagerSubsystem的外部对象
 */
UCLASS(MinimalAPI, Abstract, Blueprintable, Within = ModularUIManagerSubsystem)
class UModularUIPolicy : public UObject
{
	GENERATED_BODY()

public:
	UE_API UModularUIPolicy(const FObjectInitializer& ObjectInitializer);

	template <typename ModularUIPolicyClass = UModularUIPolicy>
	static ModularUIPolicyClass* GetModularUIPolicyAs(const UObject* WorldContextObject)
	{
		return Cast<ModularUIPolicyClass>(GetUModularUIPolicy(WorldContextObject));
	}

	/*从子系统中获取UI布局策略*/
	static UE_API UModularUIPolicy* GetUModularUIPolicy(const UObject* WorldContextObject);

public:
	UE_API virtual UWorld* GetWorld() const override;
	UE_API UModularUIManagerSubsystem* GetOwningUIManager() const;
	UE_API UPrimaryLayoutWidget* GetRootLayout(const UModularLocalPlayer* LocalPlayer) const; //从缓存的UI策略信息中获取根布局引用

	ELocalMultiplayerInteractionMode GetLocalMultiplayerInteractionMode() const { return LocalMultiplayerInteractionMode; }

	UE_API void RequestPrimaryControl(UPrimaryLayoutWidget* Layout);

protected:
	UE_API void AddLayoutToViewport(UModularLocalPlayer* LocalPlayer, UPrimaryLayoutWidget* Layout);
	UE_API void RemoveLayoutFromViewport(UModularLocalPlayer* LocalPlayer, UPrimaryLayoutWidget* Layout);

	UE_API virtual void OnRootLayoutAddedToViewport(UModularLocalPlayer* LocalPlayer, UPrimaryLayoutWidget* Layout);
	UE_API virtual void OnRootLayoutRemovedFromViewport(UModularLocalPlayer* LocalPlayer, UPrimaryLayoutWidget* Layout);
	UE_API virtual void OnRootLayoutReleased(UModularLocalPlayer* LocalPlayer, UPrimaryLayoutWidget* Layout);

	UE_API void CreateLayoutWidget(UModularLocalPlayer* LocalPlayer);
	UE_API TSubclassOf<UPrimaryLayoutWidget> GetLayoutWidgetClass(UModularLocalPlayer* LocalPlayer); //同步加载Class引用

private:
	ELocalMultiplayerInteractionMode LocalMultiplayerInteractionMode = ELocalMultiplayerInteractionMode::PrimaryOnly;

	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UPrimaryLayoutWidget> LayoutClass;

	UPROPERTY(Transient)
	TArray<FRootViewportLayoutInfo> RootViewportLayouts;

private:
	UE_API void NotifyPlayerAdded(UModularLocalPlayer* LocalPlayer);
	UE_API void NotifyPlayerRemoved(UModularLocalPlayer* LocalPlayer);
	UE_API void NotifyPlayerDestroyed(UModularLocalPlayer* LocalPlayer);

	friend class UModularUIManagerSubsystem;
};

#undef UE_API
