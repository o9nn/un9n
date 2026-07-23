// Copyright Deep Tree Echo. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

/// <summary>
/// DeepTreeEchoAvatar runtime module.
///
/// Declared in UnrealEngineCog.uproject. Canonical sources live under
/// Source/DeepTreeEchoAvatar/{Public,Private} (this directory) so that
/// UnrealBuildTool discovers and compiles them natively. The historical
/// copies under UnrealEcho/DeepTreeEchoAvatar remain the research-tree
/// mirror consumed by the standalone (non-UE) build.
///
/// Before this Build.cs existed the module was declared in the .uproject
/// with no module rules, which UnrealBuildTool rejects at project load.
/// </summary>
public class DeepTreeEchoAvatar : ModuleRules
{
	public DeepTreeEchoAvatar(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// AGIPCGManager drives procedural content generation graphs.
				"PCG",
			}
		);

		bUseUnity = false;
	}
}
