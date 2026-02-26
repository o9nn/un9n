using UnrealBuildTool;

public class TensorLogic : ModuleRules
{
    public TensorLogic(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core", 
            "CoreUObject", 
            "Engine"
        });

        PrivateDependencyModuleNames.AddRange(new string[] 
        {
        });

        // Add public include paths
        PublicIncludePaths.Add(ModuleDirectory);
    }
}
