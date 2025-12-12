// Copyright © 2025, Databiomes Inc. All rights reserved

#include "FloraEngineEditor.h"
#include "FloraEngineSettingsCustomization.h"
#include "Developer/Settings/Public/ISettingsModule.h"
#include "FloraEngineSettings.h"

#define LOCTEXT_NAMESPACE "FFloraEngineEditorModule"

void FFloraEngineEditorModule::StartupModule()
{
	// Create and register the custom pin factory
	FloraModelPinFactoryPtr = MakeShareable(new FFloraModelPinFactory());
	FEdGraphUtilities::RegisterVisualPinFactory(FloraModelPinFactoryPtr);

	// Register the custom settings
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(UFloraEngineSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FFloraEngineSettingsCustomization::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();
}

void FFloraEngineEditorModule::ShutdownModule()
{
	FEdGraphUtilities::UnregisterVisualPinFactory(FloraModelPinFactoryPtr);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_GAME_MODULE(FFloraEngineEditorModule, FloraEngineEditor)