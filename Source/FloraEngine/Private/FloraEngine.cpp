// Copyright Epic Games, Inc. All Rights Reserved.

#include "FloraEngine.h"
#include "FloraEngineSettings.h"
#include "Developer/Settings/Public/ISettingsModule.h"

#define LOCTEXT_NAMESPACE "FFloraEngineModule"

void FFloraEngineModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	//UE_LOG(LogTemp, Warning, TEXT("HELLO FROM NOT INIT"));

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
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFloraEngineModule, FloraEngine)