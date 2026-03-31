// Copyright © 2026 鸿源z. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "ModularLoadingQueryInterface.generated.h"

#define UE_API MODULARLOADINGSCREEN_API

/** 可能导致加载的界面，需要显示加载屏幕 */
UINTERFACE()
class UModularLoadingQueryInterface : public UInterface
{
	GENERATED_BODY()
};

/*用于查询需要显示加载的接口*/
class IModularLoadingQueryInterface
{
	GENERATED_BODY()

public:
	//检查传入对象是否实现接口,并且获取到是否需要显示加载屏幕
	static UE_API bool ShouldShowLoadingScreen(UObject* TestObject, FString& OutReason);

	virtual bool ShouldShowLoadingScreen(FString& OutReason) const
	{
		return false;
	}
};

/*直接内联,源码中写在 Manager 中,这里移动了一下*/
inline bool IModularLoadingQueryInterface::ShouldShowLoadingScreen(UObject* TestObject, FString& OutReason)
{
	if (TestObject == nullptr) return false;

	IModularLoadingQueryInterface* LoadObserver = Cast<IModularLoadingQueryInterface>(TestObject);
	if (!LoadObserver) return false;

	FString ObserverReason;
	if (LoadObserver->ShouldShowLoadingScreen(/*out*/ ObserverReason))
	{
		if (ensureMsgf(!ObserverReason.IsEmpty(), TEXT("%s failed to set a reason why it wants to show the loading screen"), *GetPathNameSafe(TestObject)))
			OutReason = ObserverReason;

		return true;
	}

	return false;
}

#undef UE_API
