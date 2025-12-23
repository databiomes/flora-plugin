// Copyright © 2025, Databiomes Inc. All rights reserved

#include "FloraEngine.h"
#include "FloraEngineSettings.h"
#include "Developer/Settings/Public/ISettingsModule.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FFloraEngineModule"

void FFloraEngineModule::StartupModule()
{
	FString BaseDir = IPluginManager::Get().FindPlugin("FloraEngine")->GetBaseDir();
	FString LibraryPath;

	// Determine platform and set library path accordingly
#if PLATFORM_WINDOWS	
	LibraryPath = FPaths::Combine(*BaseDir, FLORA_DLL_PATH);
#elif PLATFORM_MAC
	LibraryPath = FPaths::Combine(*BaseDir, FLORA_DYLIB_PATH);
#elif PLATFORM_LINUX || PLATFORM_ANDROID
	LibraryPath = FLORA_SO_PATH;
#endif

#if PLATFORM_WINDOWS // may need for mac and linux
	LibraryPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*LibraryPath);
#endif

	FloraDLLHandle = FPlatformProcess::GetDllHandle(*LibraryPath);

	if (!FloraDLLHandle)
	{
		UE_LOG(LogTemp, Error, TEXT("ThirdPartyLibraryError: Flora"));
		return;
	}

	// Register settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Flora_Engine_Settings",
			LOCTEXT("FloraEngine", "Flora Engine"), LOCTEXT("FloraEngineDescription", "Configure Flora Engine plugin."),
			GetMutableDefault<UFloraEngineSettings>());
	}
}

void FFloraEngineModule::ShutdownModule()
{
	if (FloraDLLHandle)
	{
		FPlatformProcess::FreeDllHandle(FloraDLLHandle);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFloraEngineModule, FloraEngine)