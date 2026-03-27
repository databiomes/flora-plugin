// Copyright © 2025, Databiomes Inc. All rights reserved

#include "FloraModelInstructionPin.h"
#include "Widgets/Input/STextComboBox.h"

#include "Interfaces/IPluginManager.h"
#include "FloraEngineSettings.h"


void SFloraModelInstructionPin::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
}

TSharedRef<SWidget>	SFloraModelInstructionPin::GetDefaultValueWidget()
{
	// Get model name from other pin on node
	UEdGraphPin* ModelPin = GraphPinObj->GetOwningNode()->FindPin(FString("ModelName"), EEdGraphPinDirection::EGPD_Input);
	if (ModelPin && ModelPin->LinkedTo.Num() > 0) {

		UE_LOG(LogTemp, Error, TEXT("Model pin must not be linked on node!"));
	}
	else {
		// Get model name from pin default value
		FString ModelPinString = ModelPin->GetDefaultAsString();

		// json file must have same name as subfolder
		FString Path = UFloraEngineSettings::GetTemplatePath(FName(ModelPinString));

		FString FileText;
		FFileHelper::LoadFileToString(FileText, *Path);

		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(FileText);
		TSharedPtr<FJsonObject> ResponseObj;
		FJsonSerializer::Deserialize(JsonReader, ResponseObj);

		// Read in options from json
		InstructionList.Empty();
		if (ResponseObj.IsValid()) {
			TSharedPtr<FJsonObject> ModelInput = ResponseObj->GetObjectField(TEXT("instructions"));

			for (auto Option : ModelInput->Values)
			{
				InstructionList.Add(MakeShared<FName>(Option.Key));
			}
		}
	}

	// Set initial value
	TSharedPtr<FName> InitialSelectedName = GetSelectedName();
	if (InitialSelectedName.IsValid())
	{
		SetPropertyWithName(*InitialSelectedName.Get());
	}

	return SAssignNew(NameComboBox, SNameComboBox)    
		.ContentPadding(FMargin(6.0f, 2.0f))         
		.OptionsSource(&InstructionList)                
		.InitiallySelectedItem(InitialSelectedName)    
		.OnComboBoxOpening(this, &SFloraModelInstructionPin::OnComboBoxOpening)
		.OnSelectionChanged(this, &SFloraModelInstructionPin::OnNameSelected);

}

void SFloraModelInstructionPin::OnNameSelected(TSharedPtr<FName> ItemSelected, ESelectInfo::Type SelectInfo)
{
	if (ItemSelected.IsValid())
	{
		SetPropertyWithName(*ItemSelected);
	}
}

void SFloraModelInstructionPin::OnComboBoxOpening()
{
	TSharedPtr<FName> SelectedName = GetSelectedName();
	if (SelectedName.IsValid())
	{
		check(NameComboBox.IsValid());
		NameComboBox->SetSelectedItem(SelectedName);
	}
}

TSharedPtr<FName> SFloraModelInstructionPin::GetSelectedName()
{
	FName Name;
	GetPropertyAsName(Name);

	// Check against list of options and return if found
	for (int32 NameIndex = 0; NameIndex < InstructionList.Num(); ++NameIndex)
	{
		if (Name == *InstructionList[NameIndex].Get())
		{
			return InstructionList[NameIndex];
		}
	}
	if (InstructionList.Num() == 0)
	{
		InstructionList.Add(MakeShareable(new FName(TEXT("No Instructions Found"))));
	}

	// No value has been found, return a default value
	return InstructionList[0];
}

void SFloraModelInstructionPin::SetPropertyWithName(FName& Name)
{
	FString PinString = *Name.ToString();

	FString CurrentDefaultValue = GraphPinObj->GetDefaultAsString();

	// If the value has changed, update the pin
	if (CurrentDefaultValue != PinString)
	{
		const FScopedTransaction Transaction(
			NSLOCTEXT("GraphEditor", "ChangeListNameFromConfigPinValue", "Change List Name From Config Value"));
		GraphPinObj->Modify();

		if (PinString != GraphPinObj->GetDefaultAsString())
		{
			GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, PinString);
		}
	}
}

void SFloraModelInstructionPin::GetPropertyAsName(FName& OutName)
{
	FString PinString = GraphPinObj->GetDefaultAsString();

	if (!PinString.IsEmpty())
	{
		OutName = *PinString;
	}
}

