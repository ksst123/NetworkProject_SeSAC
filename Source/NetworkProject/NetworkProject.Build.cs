// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NetworkProject : ModuleRules
{
	public NetworkProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "OnlineSubsystemSteam", "UMG", "OnlineSubsystem", "AnimGraphRuntime" });
    }
}