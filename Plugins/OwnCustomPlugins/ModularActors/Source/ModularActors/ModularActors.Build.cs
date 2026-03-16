// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ModularActors : ModuleRules
{
	public ModularActors(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				// ... 添加此处要求的公共包含路径 ...
			}
		);


		PrivateIncludePaths.AddRange(
			new string[]
			{
				// ... 这里需要添加其他私人包含路径 ...
			}
		);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"ModularGameplay",
				"AIModule",
				// ... 在这里添加你静态链接的其他公共依赖 ...
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ...添加你静态链接的私有依赖，在这里...	
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... 在这里添加任何你模块动态加载的模块 ...
			}
		);
	}
}