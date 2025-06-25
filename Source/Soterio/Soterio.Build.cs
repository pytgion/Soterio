// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Soterio : ModuleRules
{
	public Soterio(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"RealtimeMeshComponent",
			"Json",
			"JsonUtilities",
			"UMG",
			"AssetRegistry",
			"MeshDescription",
			"StaticMeshDescription"
		});
	}
}
