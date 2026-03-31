// Copyright © 2026, Databiomes Inc. All rights reserved

#include "FloraModelNamePin.h"
#include "Widgets/Input/STextComboBox.h"

#include "Interfaces/IPluginManager.h"
#include "FloraEngineSettings.h"


void SFloraModelNamePin::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
}

TSharedRef<SWidget>	SFloraModelNamePin::GetDefaultValueWidget()
{
	// Populate model list from Model root directory
	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> Folders;
	FileManager.FindFiles(Folders, *(UFloraEngineSettings::GetModelRootPath() + "/*"), false, true);
	for(FString& Folder : Folders) {
		ModelList.Add(MakeShareable(new FName(*Folder)));
	}

	// Set initial value
	TSharedPtr<FName> InitialSelectedName = GetSelectedName();
	if (InitialSelectedName.IsValid())
	{
		SetPropertyWithName(*InitialSelectedName.Get());
	}

	return SAssignNew(NameComboBox, SNameComboBox)    
		.ContentPadding(FMargin(6.0f, 2.0f))         
		.OptionsSource(&ModelList)                
		.InitiallySelectedItem(InitialSelectedName)    
		.OnComboBoxOpening(this, &SFloraModelNamePin::OnComboBoxOpening) 
		.OnSelectionChanged(this, &SFloraModelNamePin::OnNameSelected);

}

void SFloraModelNamePin::OnNameSelected(TSharedPtr<FName> ItemSelected, ESelectInfo::Type SelectInfo)
{
	if (ItemSelected.IsValid())
	{
		SetPropertyWithName(*ItemSelected);
	}
}

void SFloraModelNamePin::OnComboBoxOpening()
{
	TSharedPtr<FName> SelectedName = GetSelectedName();
	if (SelectedName.IsValid())
	{
		check(NameComboBox.IsValid());
		NameComboBox->SetSelectedItem(SelectedName);
	}
}

TSharedPtr<FName> SFloraModelNamePin::GetSelectedName()
{
	FName Name;
	GetPropertyAsName(Name);

	// Check against list of options and return if found
	for (int32 NameIndex = 0; NameIndex < ModelList.Num(); ++NameIndex)
	{
		if (Name == *ModelList[NameIndex].Get())
		{
			return ModelList[NameIndex];
		}
	}
	if (ModelList.Num() == 0)
	{
		ModelList.Add(MakeShareable(new FName(TEXT("No Models Found"))));
	}

	// No value has been found, return a default value
	return ModelList[0];
}

void SFloraModelNamePin::SetPropertyWithName(FName& Name)
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

void SFloraModelNamePin::GetPropertyAsName(FName& OutName)
{
	FString PinString = GraphPinObj->GetDefaultAsString();

	if (!PinString.IsEmpty())
	{
		OutName = *PinString;
	}
}

