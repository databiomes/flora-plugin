// Copyright © 2025, Databiomes Inc. All rights reserved

#pragma once
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "FloraModelPinFactory.h"


class FFloraEngineEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<FFloraModelPinFactory> FloraModelPinFactoryPtr;
};
