// Copyright © 2026, Databiomes Inc. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"
#include "FloraEngineSubsystem.h"

/*
 * Flora Model Pin Factory class.
 * Factory to create custom pins for the Flora Engine plugin.
 */
class FLORAENGINEEDITOR_API FFloraModelPinFactory : public FGraphPanelPinFactory
{
	virtual TSharedPtr<class SGraphPin> CreatePin(class UEdGraphPin* InPin) const override;
};
