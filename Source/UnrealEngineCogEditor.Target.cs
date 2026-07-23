// Copyright Deep Tree Echo. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

/// <summary>
/// Editor target for the UnrealEngineCog project. This is the target
/// used by full-engine UnrealHeaderTool validation in CI (UHT runs as
/// the first stage of an editor-target UnrealBuildTool invocation).
/// </summary>
public class UnrealEngineCogEditorTarget : TargetRules
{
	public UnrealEngineCogEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
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
