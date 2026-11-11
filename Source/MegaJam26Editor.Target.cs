using UnrealBuildTool;

public class MegaJam26EditorTarget : TargetRules
{
    public MegaJam26EditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("MegaJam26");
    }
}
