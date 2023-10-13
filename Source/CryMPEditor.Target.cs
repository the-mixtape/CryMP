// CryMP, All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class CryMPEditorTarget : TargetRules
{
	public CryMPEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;

		ExtraModuleNames.AddRange( new string[] { "CryMP" } );
	}
}
