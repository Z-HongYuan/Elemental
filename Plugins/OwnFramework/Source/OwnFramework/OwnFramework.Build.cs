// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OwnFramework : ModuleRules
{
	public OwnFramework(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
			}
		);
		PrivateIncludePaths.AddRange(
			new string[]
			{
			}
		);
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayAbilities",
			}
		);
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			}
		);
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}