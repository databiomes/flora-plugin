// Copyright © 2025, Databiomes Inc. All rights reserved

#pragma once

#include "Modules/ModuleManager.h"

class FFloraEngineModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void* GetFloraDLLHandle() const { return FloraDLLHandle; }

	void FreeFloraDLLHandle();
private:
	void* FloraDLLHandle;
};
