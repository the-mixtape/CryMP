// CryMP, All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class CryMPTarget : TargetRules
{
	public CryMPTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;

		ExtraModuleNames.AddRange( new string[] { "CryMP" } );
	}
}
