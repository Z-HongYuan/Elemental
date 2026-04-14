// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogModularUI, Log, All); //注册Log分类
DEFINE_LOG_CATEGORY(LogModularUI); //注册Log分类

#define LOCTEXT_NAMESPACE "FModularUIModule"

class FModularUIModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FModularUIModule::StartupModule()
{
}

void FModularUIModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularUIModule, ModularUI)

/*
 * 针对于CommonUI的拓展
 * 融合了标签系统,实现了模块化UI系统
 * 基本使用方法:
 * 1. 继承 UModularUIPolicy 类,并且设置其中需要使用的Root根控件
 * 2. 设置Manager中的UIPolicy的引用
 * 3. 问题:考虑在 NotifyPlayerAdded() 函数中的调用时序问题,Lyra是使用的 ReceivedPlayer() 时机
 * 4. 继承并使用Root根控件类
 * 
 */
