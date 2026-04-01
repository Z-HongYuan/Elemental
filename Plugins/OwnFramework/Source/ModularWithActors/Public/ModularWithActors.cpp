// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FModularWithActorsModule"

class FModularWithActorsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularWithActorsModule, ModularWithActors)

/*
 * 使用方法:
 * 在蓝图,或者C++中,直接继承模块化最小对象即可
 * 模块化最小对象(基础):
 * 1. 在 PreInitializeComponents 预初始化组件之后注册为动态组件的接收者
 * 2. 在 BeginPlay 发布GameActorReady事件
 * 3. 在 EndPlay 移除动态组件的接收者
 * 在特殊对象中还拥有额外逻辑:
 * 1. 例如控制器会额外连锁调用对应组件的函数
 * 此类需要暴露API
 */
