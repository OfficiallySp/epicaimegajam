using UnrealBuildTool;

public class MegaJam26 : ModuleRules
{
    public MegaJam26(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicIncludePaths.Add(ModuleDirectory); // sources include as "Sim/...", "Game/...", "UI/..."
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "ApplicationCore", "Slate", "SlateCore" });
    }
}
