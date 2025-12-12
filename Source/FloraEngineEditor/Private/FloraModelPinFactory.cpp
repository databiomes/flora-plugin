// Copyright © 2025, Databiomes Inc. All rights reserved

#include "FloraModelPinFactory.h"
#include "FloraModelNamePin.h"
#include "FloraModelInstructionPin.h"

TSharedPtr<class SGraphPin> FFloraModelPinFactory::CreatePin(class UEdGraphPin* InPin) const
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	
	// Check if pin is a Name and named InstructionLine
	if (InPin->PinType.PinCategory == K2Schema->PC_Name && InPin->PinName == "InstructionLine") {
		return SNew(SFloraModelInstructionPin, InPin);
	}
	// Check if pin is a Name and named ModelName
	else if(InPin->PinType.PinCategory == K2Schema->PC_Name && InPin->PinName == "ModelName") {
		return SNew(SFloraModelNamePin, InPin); 
	}
	return nullptr;

}