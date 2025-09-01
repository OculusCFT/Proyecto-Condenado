using UnrealBuildTool;

public class MyProject : ModuleRules
{
	public MyProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"HeadMountedDisplay",
			"XRBase",
			"NavigationSystem",
			"Niagara",
			"EnhancedInput"
		});

		PrivateDependencyModuleNames.AddRange(new string[] 
		{
			"Slate",
			"SlateCore",
			"UMG"
		});
		
		// Para VR
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicDependencyModuleNames.AddRange(new string[]
			{
				"D3D11RHI",
				"D3D12RHI"
			});
		}
	}
}