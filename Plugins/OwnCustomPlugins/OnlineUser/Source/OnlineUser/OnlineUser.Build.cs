// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OnlineUser : ModuleRules
{
	public OnlineUser(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				// ... add public include paths required here ...
			}
		);


		PrivateIncludePaths.AddRange(
			new string[]
			{
				// ... add other private include paths required here ...
			}
		);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayTags"

				// ... add other public dependencies that you statically link with here ...
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"OnlineSubsystemUtils"
				// ... add private dependencies that you statically link with here ...	
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);

		/*根据条件编译判断使用那个版本的OSS*/
		bool bUseOnlineSubsystemV1 = true;

		if (bUseOnlineSubsystemV1)
		{
			PublicDependencyModuleNames.Add("OnlineSubsystem");
		}
		else
		{
			PublicDependencyModuleNames.Add("OnlineServicesInterface");
		}

		/*定义宏,能够使用 #if ONLINEUSER_OSSV1 进行条件编译*/
		PublicDefinitions.Add("ONLINEUSER_OSSV1=" + (bUseOnlineSubsystemV1 ? "1" : "0"));
	}
}