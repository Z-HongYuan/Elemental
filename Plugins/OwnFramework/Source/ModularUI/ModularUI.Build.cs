// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ModularUI : ModuleRules
{
	public ModularUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CommonUI",
				"GameplayTags",
				"UMG",
				"CommonInput"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore"
			}
		);
	}
}