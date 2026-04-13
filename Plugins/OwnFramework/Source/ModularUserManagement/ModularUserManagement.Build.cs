// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ModularUserManagement : ModuleRules
{
	public ModularUserManagement(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayTags",
				"OnlineSubsystemUtils",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"CoreOnline",
				"Engine",
				"Slate",
				"SlateCore",
				"OnlineSubsystemUtils",
				"ApplicationCore",
				"InputCore",
			}
		);

		bool bUseOnlineSubsystemV1 = true;
		if (bUseOnlineSubsystemV1)
		{
			PublicDependencyModuleNames.Add("OnlineSubsystem");
		}
		else
		{
			PublicDependencyModuleNames.Add("OnlineServicesInterface");
		}

		PublicDefinitions.Add("MODULARUSER_OSSV1=" + (bUseOnlineSubsystemV1 ? "1" : "0"));
	}
}