// Copyright Epic Games, Inc. All Rights Reserved.


#include "FloraModelInputPin.h"
#include "Widgets/Input/STextComboBox.h"

#include "Interfaces/IPluginManager.h"
#include "FloraEngineSettings.h"


void SFloraModelInputPin::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	LineList.Empty();
	// Index of the model input options in the json file (assigned in the factory)
	JsonIndex = InArgs._Index;

	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
}
TSharedRef<SWidget>	SFloraModelInputPin::GetDefaultValueWidget()
{
	// Get model name from other pin on node
	UEdGraphPin* ModelPin = GraphPinObj->GetOwningNode()->FindPin(FString("ModelName"), EEdGraphPinDirection::EGPD_Input);
	if (ModelPin && ModelPin->LinkedTo.Num() > 0) {

		UE_LOG(LogTemp, Error, TEXT("Model pin must not be linked on node!"));
	}
	else {
		FString BaseDir = IPluginManager::Get().FindPlugin("FloraEngine")->GetBaseDir();

		FString TokenizerPath;
		TokenizerPath = FPaths::Combine(*BaseDir, UFloraEngineSettings::GetModelRootPath());

		// Get model name from pin default value
		FString ModelPinString = ModelPin->GetDefaultAsString();

		// json file must have same name as subfolder
		FString Path = FPaths::Combine(*TokenizerPath, ModelPinString + "/" + ModelPinString + ".json");

		FString FileText;
		FFileHelper::LoadFileToString(FileText, *Path);

		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(FileText);
		TSharedPtr<FJsonObject> ResponseObj;
		FJsonSerializer::Deserialize(JsonReader, ResponseObj);

		// Read in options from json
		LineList.Empty();
		if (ResponseObj.IsValid()) {
			TSharedPtr<FJsonObject> ModelInput = ResponseObj->GetObjectField(TEXT("all_combinations"))->GetObjectField(TEXT("model_input"));
			TArray<TSharedPtr<FJsonValue>> Options = ModelInput->GetArrayField(FString::FromInt(JsonIndex));

			for (TSharedPtr<FJsonValue> Option : Options)
			{
				FString Left, Right;
				Option->AsString().Split(": ", &Left, &Right);;
				LineList.Add(MakeShared<FString>(Left));
			}
		}
	}

	// Set initial value
	TSharedPtr<FString> InitialSelectedValue = GetSelectedModelLine();
	if (InitialSelectedValue.IsValid())
	{
		SetPropertyWithString(*InitialSelectedValue);
	}

	return	SNew(STextComboBox)
		.OptionsSource(&LineList)
		.InitiallySelectedItem(InitialSelectedValue)
		.OnSelectionChanged(this, &SFloraModelInputPin::OnLineSelected);

}

void SFloraModelInputPin::OnLineSelected(TSharedPtr<FString> ItemSelected, ESelectInfo::Type SelectInfo)
{
	SetPropertyWithString(*ItemSelected.Get());
}

TSharedPtr<FString> SFloraModelInputPin::GetSelectedModelLine()
{
	// Parse pin value to get current selection
	FString PinString = GraphPinObj->GetDefaultAsString();
	if (PinString.StartsWith(TEXT("(")) && PinString.EndsWith(TEXT(")")))
	{
		PinString = PinString.LeftChop(1);
		PinString = PinString.RightChop(1);
		PinString.Split("=", NULL, &PinString);
		PinString.Split(",", &PinString, NULL);

		if (PinString.StartsWith(TEXT("\"")) && PinString.EndsWith(TEXT("\"")))
		{
			PinString = PinString.LeftChop(1);
			PinString = PinString.RightChop(1);
		}
	}

	// Check against list of options
	for (int32 LineIndex = 0; LineIndex < LineList.Num(); LineIndex++)
	{
		if (PinString == *LineList[LineIndex].Get())
		{
			return LineList[LineIndex];
		}
	}

	if (LineList.Num() == 0)
	{
		LineList.Add(MakeShareable(new FString(TEXT("No Models Found"))));
	}

	// Default to first option if current value not found
	return LineList[0];
}

void SFloraModelInputPin::SetPropertyWithString(FString String)
{
	// Set pin value to new selection
	// Format is (ModelLine="line text",LineIndex=index)
	FString PinString = TEXT("(ModelLine=\"");
	PinString += String;
	PinString += TEXT("\",LineIndex=");
	for (int32 LineIndex = 0; LineIndex < LineList.Num(); LineIndex++)
	{
		if (*LineList[LineIndex] == String)
		{
			PinString += FString::FromInt(LineIndex);
			break;
		}
	}
	PinString += TEXT(")");

	// If value has changed, update it
	FString CurrentDefaultValue = GraphPinObj->GetDefaultAsString();
	if (CurrentDefaultValue != PinString)
	{
		const FScopedTransaction Transaction(
			NSLOCTEXT("GraphEditor", "ChangeListNameFromConfigPinValue", "Change List Name From Config Value"));
		GraphPinObj->Modify();

		//UE_LOG(LogTemp, Warning, TEXT("Verify values old: \"%s\" chosen: \"%s\""), *CurrentDefaultValue, *PinString);
		if (PinString != GraphPinObj->GetDefaultAsString())
		{
			GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, PinString);
		}
	}
}