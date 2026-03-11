// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LoadingQueryInterface.generated.h"

#define UE_API LOADINGSCREEN_API

/*
 * 用于处理可能导致加载操作并需要显示加载屏幕的事项的接口
 */
UINTERFACE(MinimalAPI, BlueprintType)
class ULoadingQueryInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 用于处理可能导致加载操作并需要显示加载屏幕的条件接口
 */
class ILoadingQueryInterface
{
	GENERATED_BODY()

public:
	/*
	 * 检查此对象是否实现了接口，如果是，则询问我们当前是否应显示加载屏幕
	 * @Param TestObject 要检查的对象
	 * @Param OutReason 显示加载界面的原因
	 */
	static UE_API bool ShouldShowLoadingScreen(UObject* TestObject, FString& OutReason);

	virtual bool ShouldShowLoadingScreen(FString& OutReason) const
	{
		return false;
	}
};

/*
 * 此函数,在原文写在LoadingScreenManager的cpp中,但是我感觉移动到这里比较好,比较直观能看
 */
inline bool ILoadingQueryInterface::ShouldShowLoadingScreen(UObject* TestObject, FString& OutReason)
{
	if (TestObject == nullptr) return false;

	if (ILoadingQueryInterface* LoadObserver = Cast<ILoadingQueryInterface>(TestObject))
	{
		FString ObserverReason;
		if (LoadObserver->ShouldShowLoadingScreen(/*out*/ ObserverReason))
		{
			if (ensureMsgf(!ObserverReason.IsEmpty(), TEXT("%s 这里是未能设置其想要显示加载屏幕的原因"), *GetPathNameSafe(TestObject)))
			{
				OutReason = ObserverReason;
			}
			return true;
		}
	}

	return false;
}


#undef UE_API
