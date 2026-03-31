// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ModularLoadingScreen : ModuleRules
{
    public ModularLoadingScreen(ReadOnlyTargetRules Target) : base(Target)
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
                "Engine",
                "Slate",
                "SlateCore", 
                "PreLoadScreen",
                "DeveloperSettings",
                "RenderCore",
                "UMG",
            }
        );
    }
}