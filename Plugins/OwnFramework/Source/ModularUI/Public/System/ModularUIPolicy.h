// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "Widgets/ModularRootLayoutWidget.h"
#include "ModularUIPolicy.generated.h"

#define UE_API MODULARUI_API

class UModularUIManager;

/*代表着,当前游戏中是单人单屏,还是多人单屏,多人单屏分为分屏模式和切屏模式*/
UENUM()
enum class ELocalMultiplayerViewMode : uint8
{
	// 仅适用于主要玩家的全屏视口，而不管其他玩家是否存在
	PrimaryOnly,

	// 一个玩家的全屏视口，但玩家可以交换对谁显示谁处于休眠状态的控制权
	SingleToggle,

	// 同时为两名玩家显示的视口
	Simultaneous
};

/*代表着一个本地玩家所拥有的信息*/
USTRUCT()
struct FRootViewportLayoutInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer = nullptr; //代表的本地玩家

	UPROPERTY(Transient)
	TObjectPtr<UModularRootLayoutWidget> RootLayoutWidget = nullptr; //代表的根布局

	UPROPERTY(Transient)
	bool bAddedToViewport = false; //是否添加到视口

	FRootViewportLayoutInfo()
	{
	}

	FRootViewportLayoutInfo(ULocalPlayer* InLocalPlayer, UModularRootLayoutWidget* InRootLayout, bool bIsInViewport)
		: LocalPlayer(InLocalPlayer)
		  , RootLayoutWidget(InRootLayout)
		  , bAddedToViewport(bIsInViewport)
	{
	}

	bool operator==(const ULocalPlayer* OtherLocalPlayer) const { return LocalPlayer == OtherLocalPlayer; }
};

/**
 * 针对于Root根控件的管理/策略
 */
UCLASS(MinimalAPI, Abstract, Blueprintable, Within=ModularUIManager)
class UModularUIPolicy : public UObject
{
	GENERATED_BODY()

public:
	/*获取 UIPolicy 的静态模版函数,能够转换为其他子型*/
	template <typename GameUIPolicyClass = UModularUIPolicy>
	static GameUIPolicyClass* GetModularUIPolicy(const UObject* WorldContextObject)
	{
		return Cast<GameUIPolicyClass>(GetModularUIPolicy(WorldContextObject));
	}

	/*获取 UIPolicy 的静态函数*/
	static UE_API UModularUIPolicy* GetModularUIPolicy(const UObject* WorldContextObject);

	UE_API virtual UWorld* GetWorld() const override;

	//因为创建此Obj的外部类,必然是 UModularUIManager 所以可以直接返回 Outer(),而且确定是使用了Within说明符
	UE_API UModularUIManager* GetOwningUIManager() const;

	//获取本地玩家所拥有的Root根控件引用
	UE_API UModularRootLayoutWidget* GetModularRootLayoutWidget(const ULocalPlayer* LocalPlayer) const;

	//获取视口模式,详情请参阅 ELocalMultiplayerViewMode ,默认为 单人单屏
	ELocalMultiplayerViewMode GetLocalMultiplayerViewMode() const { return LocalMultiplayerViewMode; }

	//请求传入的Root根控件激活,并将其他玩家的控件停用
	UE_API void RequestPrimaryControl(UModularRootLayoutWidget* Layout);

protected:
	//事件通知函数
	UE_API virtual void OnRootLayoutAddedToViewport(ULocalPlayer* LocalPlayer, UModularRootLayoutWidget* Layout);
	UE_API virtual void OnRootLayoutRemovedFromViewport(ULocalPlayer* LocalPlayer, UModularRootLayoutWidget* Layout);
	UE_API virtual void OnRootLayoutReleased(ULocalPlayer* LocalPlayer, UModularRootLayoutWidget* Layout);

	//获取 默认使用的Root根控件类,同步是因为这个是必须要加载的
	UE_API TSubclassOf<UModularRootLayoutWidget> GetLayoutWidgetClass() const { return LayoutClass.LoadSynchronous(); }
	UE_API void CreateLayoutWidgetAndAddToViewport(ULocalPlayer* LocalPlayer);

private:
	//内部使用的添加/移除操作,附带了事件通知函数调用
	UE_API void AddLayoutToViewport(ULocalPlayer* LocalPlayer, UModularRootLayoutWidget* Layout);
	UE_API void RemoveLayoutFromViewport(ULocalPlayer* LocalPlayer, UModularRootLayoutWidget* Layout);

	//从 UModularUIManager 传递的函数调用事件,源头是 GameInstance
	friend class UModularUIManager;
	UE_API void NotifyPlayerAdded(ULocalPlayer* LocalPlayer);
	UE_API void NotifyPlayerRemoved(ULocalPlayer* LocalPlayer);
	UE_API void NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer);

	//要构建和使用的Root根控件路径
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UModularRootLayoutWidget> LayoutClass;

	UPROPERTY(Transient)
	TArray<FRootViewportLayoutInfo> RootViewportLayouts; //所有本地玩家的信息结构体

	ELocalMultiplayerViewMode LocalMultiplayerViewMode = ELocalMultiplayerViewMode::PrimaryOnly;
};

#undef UE_API
