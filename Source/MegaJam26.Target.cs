using UnrealBuildTool;

public class MegaJam26Target : TargetRules
{
    public MegaJam26Target(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("MegaJam26");
    }
}
