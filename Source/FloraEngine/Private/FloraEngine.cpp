// Copyright © 2025, Databiomes Inc. All rights reserved

#include "FloraEngine.h"
#include "FloraEngineSettings.h"
#include "Developer/Settings/Public/ISettingsModule.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/FileManager.h"

#define LOCTEXT_NAMESPACE "FFloraEngineModule"

void FFloraEngineModule::StartupModule()
{
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

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFloraEngineModule, FloraEngine)