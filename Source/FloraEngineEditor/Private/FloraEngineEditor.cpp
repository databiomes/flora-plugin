// Copyright Epic Games, Inc. All Rights Reserved.

#include "FloraEngineEditor.h"

#define LOCTEXT_NAMESPACE "FFloraEngineEditorModule"

void FFloraEngineEditorModule::StartupModule()
{
	// Create and register the custom pin factory
	FloraModelPinFactoryPtr = MakeShareable(new FFloraModelPinFactory());
	FEdGraphUtilities::RegisterVisualPinFactory(FloraModelPinFactoryPtr);
}

void FFloraEngineEditorModule::ShutdownModule()
{
	FEdGraphUtilities::UnregisterVisualPinFactory(FloraModelPinFactoryPtr);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_GAME_MODULE(FFloraEngineEditorModule, FloraEngineEditor)