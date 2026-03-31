// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ModularLoadingScreenForStartup : ModuleRules
{
    public ModularLoadingScreenForStartup(ReadOnlyTargetRules Target) : base(Target)
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
                "Core",
                "CoreUObject",
                "Slate",
                "SlateCore",
                "PreLoadScreen",
            }
        );
    }
}