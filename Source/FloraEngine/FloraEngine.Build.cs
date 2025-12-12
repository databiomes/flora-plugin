// Copyright © 2025, Databiomes Inc. All rights reserved

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

        //if (Target.Platform == UnrealTargetPlatform.Android) // Used to request audio recording permission on Android
        //{
        //    PrivateDependencyModuleNames.Add("AndroidPermission");
        //}

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

        // Define paths for different platforms
        // Flora DLL/SO/Dylib paths
        string FloraDllPath = Path.Combine("Binaries", "Win64", "flora.dll");
        FloraDllPath = FloraDllPath.Replace(@"\", @"/");
        PublicDefinitions.Add($"FLORA_DLL_PATH=\"{FloraDllPath}\"");

        string DylibPath = "Content/flora.dylib";
        PublicDefinitions.Add($"FLORA_DYLIB_PATH=\"{DylibPath}\"");

        string FloraSoPath = "libflora.so";
        PublicDefinitions.Add($"FLORA_SO_PATH=\"{FloraSoPath}\"");

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // Add Flora DLL as a runtime dependency
            RuntimeDependencies.Add(Path.Combine(PluginDirectory, FloraDllPath), Path.Combine(PluginDirectory, "Source/ThirdParty/bin/Win64/flora.dll"));
            PublicDelayLoadDLLs.Add("flora.dll");
            //System.Console.WriteLine("FloraEngine: Added RuntimeDependency for " + FloraDllPath);
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            RuntimeDependencies.Add(Path.Combine(PluginDirectory, DylibPath));
            System.Console.WriteLine("FloraEngine: Added RuntimeDependency for " + DylibPath);
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.Android)
        {
            // For Android, we need to add the APL file to the build receipt for .so packaging
            string BuildPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(BuildPath, "Flora_APL.xml")); 
            //RuntimeDependencies.Add(Path.Combine(PluginDirectory,"Content", SoPath));
        }

        // Check for ModelRootPath in DefaultEngine.ini (this value is set in FloraEngineSettings)
        string ProjectDir = Target.ProjectFile != null ? Target.ProjectFile.Directory.FullName : null;
        string ModelRootPath = "Content/Models"; // default path from FloraEngineSettings
        if (ProjectDir != null)
        {
            string DefaultGameIni = Path.Combine(ProjectDir, "Config", "DefaultEngine.ini");
            if (File.Exists(DefaultGameIni))
            {
                foreach (var line in File.ReadAllLines(DefaultGameIni))
                {
                    if (line.StartsWith("ModelRootPath="))
                    {
                        ModelRootPath = line.Split('=')[2].Trim();
                        ModelRootPath = ModelRootPath.Substring(1, ModelRootPath.Length - 3); // remove quotes and trailing )
                    }
                }
            }
        }

        // Add all files in ModelRootPath as runtime dependencies to be packaged
        RuntimeDependencies.Add(Path.Combine(PluginDirectory, ModelRootPath, "*"));
    }

    private void AddFilesInFolderToRuntimeDependencies(string Folder)
    {
        string[] FilesInSubFolder = Directory.GetFiles(Path.Combine(PluginDirectory, Folder));

        foreach (string File in FilesInSubFolder)
        {
            string FileName = Path.GetFileName(File);
            RuntimeDependencies.Add(Path.Combine(PluginDirectory, Folder, FileName));
            //System.Console.WriteLine("FloraEngine: Added RuntimeDependency for " + FileName);
        }
    }
}
