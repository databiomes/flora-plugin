// Copyright Epic Games, Inc. All Rights Reserved.


#include "FloraModelPinFactory.h"
#include "FloraModelNamePin.h"

TSharedPtr<class SGraphPin> FFloraModelPinFactory::CreatePin(class UEdGraphPin* InPin) const
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	// Check if pin is struct, and then check if that pin is of struct type we want customize	
	if (InPin->PinType.PinCategory == K2Schema->PC_Struct && InPin->PinType.PinSubCategoryObject == FModelInputLine::StaticStruct())
	{
		// For each input line pin, assign the correct index based on pin name
		if (InPin->PinName == "InputLine1") {
			return SNew(SFloraModelInputPin, InPin)
				.Index(0);
		}else if (InPin->PinName == "InputLine2") {
			return SNew(SFloraModelInputPin, InPin)
				.Index(1);
		}else if (InPin->PinName == "InputLine3") {
			return SNew(SFloraModelInputPin, InPin)
				.Index(2);
		}else {
			return SNew(SFloraModelInputPin, InPin)
				.Index(0);
		}
	} 
	// Check if pin is a Name and named ModelName
	else if(InPin->PinType.PinCategory == K2Schema->PC_Name && InPin->PinName == "ModelName") {
		return SNew(SFloraModelNamePin, InPin); 
	}
	return nullptr;

}