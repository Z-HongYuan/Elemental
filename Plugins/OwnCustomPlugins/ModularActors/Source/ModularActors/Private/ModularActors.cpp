// Copyright Epic Games, Inc. All Rights Reserved.

#include "ModularActors.h"

#define LOCTEXT_NAMESPACE "FModularActorsModule"

void FModularActorsModule::StartupModule()
{
	// 这些代码将在你的模块加载到内存后执行;具体时间点在每个模块的 .uplugin 文件中指定
}

void FModularActorsModule::ShutdownModule()
{
	// 关机时可能会调用这个功能来清理模块。 支持动态加载的模块,
	// 我们调用该函数，然后再解载模块。
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularActorsModule, ModularActors)
