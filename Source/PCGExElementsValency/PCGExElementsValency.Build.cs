// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

using System;
using System.IO;
using UnrealBuildTool;

public class PCGExElementsValency : ModuleRules
{
	public PCGExElementsValency(ReadOnlyTargetRules Target) : base(Target)
	{
		bool bNoPCH = Environment.GetEnvironmentVariable("PCGEX_NO_PCH") == "1" || File.Exists(Path.Combine(ModuleDirectory, "..", "..", "Config", ".noPCH"));
		PCHUsage = bNoPCH ? PCHUsageMode.NoPCHs : PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = true;
		MinSourceFilesForUnityBuildOverride = 4;
		PrecompileForTargets = PrecompileTargetsType.Any;
		ShortName = "PCGExValency";

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"PCG",
				"PCGExCore",
				"PCGExProperties",
				"PCGExBlending",
				"PCGExFilters",
				"PCGExFoundations",
				"PCGExGraphs",
				"PCGExCollections",
				"PCGExElementsProbing",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
