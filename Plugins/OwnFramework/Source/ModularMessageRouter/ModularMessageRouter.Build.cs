// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ModularMessageRouter : ModuleRules
{
	public ModularMessageRouter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				/*这些是基础依赖*/
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
			}
		);
	}
}