// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ZeroProtocol : ModuleRules
{
	public ZeroProtocol(ReadOnlyTargetRules Target) : base(Target)
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
			"MetasoundEngine",
			"Niagara",
			"MultiplayerSessions",
			"OnlineSubsystem",
			"OnlineSubsystemSteam"
		});

		PublicIncludePaths.AddRange(new string[] {
			"ZeroProtocol",
			"ZeroProtocol/Variant_Platforming",
			"ZeroProtocol/Variant_Platforming/Animation",
			"ZeroProtocol/Variant_Combat",
			"ZeroProtocol/Variant_Combat/AI",
			"ZeroProtocol/Variant_Combat/Animation",
			"ZeroProtocol/Variant_Combat/Gameplay",
			"ZeroProtocol/Variant_Combat/Interfaces",
			"ZeroProtocol/Variant_Combat/UI",
			"ZeroProtocol/Variant_SideScrolling",
			"ZeroProtocol/Variant_SideScrolling/AI",
			"ZeroProtocol/Variant_SideScrolling/Gameplay",
			"ZeroProtocol/Variant_SideScrolling/Interfaces",
			"ZeroProtocol/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
