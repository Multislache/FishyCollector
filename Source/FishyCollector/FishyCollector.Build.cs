// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FishyCollector : ModuleRules
{
	public FishyCollector(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"CableComponent"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"FishyCollector",
			"FishyCollector/Variant_Platforming",
			"FishyCollector/Variant_Platforming/Animation",
			"FishyCollector/Variant_Combat",
			"FishyCollector/Variant_Combat/AI",
			"FishyCollector/Variant_Combat/Animation",
			"FishyCollector/Variant_Combat/Gameplay",
			"FishyCollector/Variant_Combat/Interfaces",
			"FishyCollector/Variant_Combat/UI",
			"FishyCollector/Variant_SideScrolling",
			"FishyCollector/Variant_SideScrolling/AI",
			"FishyCollector/Variant_SideScrolling/Gameplay",
			"FishyCollector/Variant_SideScrolling/Interfaces",
			"FishyCollector/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
