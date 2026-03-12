// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameplayMessageManagerNodes : ModuleRules
{
	public GameplayMessageManagerNodes(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"BlueprintGraph",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"KismetCompiler",
				"PropertyEditor",
				"GameplayMessageManager",
				"UnrealEd"
			}
		);
	}
}