// Some copyright should be here...

using System.IO;
using UnrealBuildTool;

public class FloraEngine : ModuleRules
{
    public FloraEngine(ReadOnlyTargetRules Target) : base(Target)
    {
        PrecompileForTargets = PrecompileTargetsType.Any;
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
				// ... add public include paths required here ...
			}
            );


        PrivateIncludePaths.AddRange(
            new string[] {
				// ... add other private include paths required here ...
			}
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Projects",
				// ... add other public dependencies that you statically link with here ...
			}
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Json"
				//"UnrealEd",
				// ... add private dependencies that you statically link with here ...	
			}
            );

        if (Target.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "UnrealEd"
            }
            );
        }

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );


        PublicDelayLoadDLLs.Add("flora.dll");

		// Define paths for different platforms
		string DllPath = "Content/flora.dll";
        PublicDefinitions.Add($"FLORA_DLL_PATH=\"{DllPath}\"");

        string DylibPath = "Content/flora.dylib";
        PublicDefinitions.Add($"FLORA_DYLIB_PATH=\"{DylibPath}\"");

        string SoPath = "libflora.so";
        PublicDefinitions.Add($"FLORA_SO_PATH=\"{SoPath}\"");

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            RuntimeDependencies.Add(Path.Combine(PluginDirectory, DllPath));
            System.Console.WriteLine("FloraEngine: Added RuntimeDependency for " + Path.Combine(PluginDirectory, DllPath));
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            RuntimeDependencies.Add(Path.Combine(PluginDirectory, DylibPath));
            System.Console.WriteLine("FloraEngine: Added RuntimeDependency for " + Path.Combine(PluginDirectory, DylibPath));
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.Android)
        {
            // For Android, we need to add the APL file to the build receipt for .so packaging
            string BuildPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(BuildPath, "Flora_APL.xml")); 
            RuntimeDependencies.Add(Path.Combine(PluginDirectory,"Content", SoPath));
        }

        // Check for ModelRootPath in DefaultEngine.ini (this value is set in FloraEngineSettings)
        string ProjectDir = Target.ProjectFile != null ? Target.ProjectFile.Directory.FullName : null;
		bool found = false;
        string ModelRootPath = "";
        if (ProjectDir != null)
        {
            string DefaultGameIni = Path.Combine(ProjectDir, "Config", "DefaultEngine.ini");
            if (File.Exists(DefaultGameIni))
            {
                foreach (var line in File.ReadAllLines(DefaultGameIni))
                {
                    if (line.StartsWith("ModelRootPath="))
                    {
						found = true;
                        ModelRootPath = line.Split('=')[1].Trim();
						break;
                    }
                }
            }
        }

		if (!found)
            ModelRootPath = "Content/Models"; // default path from FloraEngineSettings

        // Add all files in ModelRootPath as runtime dependencies to be packaged
        RuntimeDependencies.Add(Path.Combine(PluginDirectory, ModelRootPath, "*"));
    }
}
