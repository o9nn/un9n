// Copyright Deep Tree Echo. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

/// <summary>
/// Game target for the UnrealEngineCog project. Required by
/// UnrealBuildTool for any cooked/game build and for UnrealHeaderTool
/// validation passes over the project's modules.
/// </summary>
public class UnrealEngineCogTarget : TargetRules
{
	public UnrealEngineCogTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;

		ExtraModuleNames.AddRange(
			new string[]
			{
				"DeepTreeEcho",
				"UnrealEcho",
				"DeepTreeEchoAvatar",
			}
		);
	}
}
