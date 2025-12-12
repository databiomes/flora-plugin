// Copyright © 2025, Databiomes Inc. All rights reserved

#include "FloraEngineSettingsCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SDirectoryPicker.h"
#include "IContentBrowserSingleton.h"
#include "FloraEngineSettings.h"
#include "Interfaces/IPluginManager.h"

#include "GenericPlatform/GenericPlatformFile.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "Kismet2/EnumEditorUtils.h"

#define LOCTEXT_NAMESPACE "FFloraEngineSettingsCustomization"

TSharedRef<IDetailCustomization> FFloraEngineSettingsCustomization::MakeInstance()
{
	return MakeShareable(new FFloraEngineSettingsCustomization());
}

FFloraEngineSettingsCustomization::FFloraEngineSettingsCustomization()
{
}

FFloraEngineSettingsCustomization::~FFloraEngineSettingsCustomization()
{

}

void FFloraEngineSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	// Find the category to replace the GenerateReactions property with a button
	IDetailCategoryBuilder& FloraEngineCategory = DetailLayout.EditCategory("General");
	{
		TArray<TSharedRef<IPropertyHandle>> FloraEngineProperties;
		FloraEngineCategory.GetDefaultProperties(FloraEngineProperties, true, true);

		// Hide the original property
		TSharedPtr<IPropertyHandle> GenerateReactionsProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UFloraEngineSettings, GenerateReactions));
		GenerateReactionsProperty->MarkHiddenByCustomization();

		for (TSharedPtr<IPropertyHandle> Property : FloraEngineProperties)
		{
			// Replace the GenerateReactions property with a button that opens a directory picker and enum generator button
			if (Property->GetProperty() == GenerateReactionsProperty->GetProperty())
			{
				FloraEngineCategory.AddCustomRow(GenerateReactionsProperty->GetPropertyDisplayName(), /*bForAdvanced*/false)
					.NameContent()
					[
						GenerateReactionsProperty->CreatePropertyNameWidget()
					]
					.ValueContent()
					[
						SNew(SButton)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.OnClicked_Lambda([this]() // Open window with directory picker and generate button
								{
									const TSharedRef<SWindow> Window = SNew(SWindow)
										.Title(LOCTEXT("GenerateReactionsTitle", "Generate Reactions"))
										.SizingRule(ESizingRule::Autosized)
										//.ClientSize(FVector2D(700, 125))
										.SupportsMinimize(false)
										.Content()
										[
											SNew(SVerticalBox)
												// Row: Directory selector and label
												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(10)
												[
													SNew(SHorizontalBox)

														+ SHorizontalBox::Slot()
														.AutoWidth()
														.VAlign(VAlign_Center)
														[
															SNew(STextBlock)
																.Text(FText::FromString("Reaction enum destination: "))
														]

														+ SHorizontalBox::Slot()
														.FillWidth(1.0f)
														.Padding(10, 0, 0, 0)
														[
															// Directory picker
															SNew(SDirectoryPicker)
																.Directory(FPaths::Combine(FPaths::ProjectContentDir(), "Reactions")) // Default directory
																.OnDirectoryChanged_Lambda([this](const FString& NewDir)
																	{
																		DestinationDirectory = NewDir;
																	})
														]
												]
										
												// Row: Checkbox for instruction reactions
												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(10)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot()
														.AutoWidth()
														.VAlign(VAlign_Center)
														[
															SNew(STextBlock)
																.Text(FText::FromString("Generate instruction specific reactions: "))
														]
														+ SHorizontalBox::Slot()
														.FillWidth(1.0f)
														.Padding(10, 0, 0, 0)
														[
															// Checkbox for instruction reactions
															SNew(SCheckBox)
																.IsChecked_Lambda([this]() -> ECheckBoxState
																	{
																		return bGenerateInstructionReactions ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
																	})
																.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
																	{
																		bGenerateInstructionReactions = (NewState == ECheckBoxState::Checked);
																	})
														]

												]

											// Generate Button in popup window
											+ SVerticalBox::Slot()
												.AutoHeight()
												.HAlign(HAlign_Center)
												.Padding(10)
												[
													SNew(SButton)
														.Text(FText::FromString("Generate"))
														.OnClicked_Lambda([this]() -> FReply
															{
																OnGenerateReactionsClicked();
																return FReply::Handled();
															})
												]
										];

									// Add the window to the application
									TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
									if (RootWindow.IsValid())
									{
										FSlateApplication::Get().AddWindowAsNativeChild(Window, RootWindow.ToSharedRef());
									}
									else
									{
										FSlateApplication::Get().AddWindow(Window);
									}

									return FReply::Handled();
								})
							[
								// Button in settings menu
								SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(FMargin(0, 0, 4, 0))
									[
										SNew(SImage)
											.Image(FAppStyle::GetBrush("Icons.Settings"))
											.ColorAndOpacity(FSlateColor::UseForeground())
									]
									+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SNew(STextBlock)
											.Text(LOCTEXT("GenerateReactions", "Generate Reactions..."))
									]
							]
					];
			}
		}
	}
}

void FFloraEngineSettingsCustomization::OnGenerateReactionsClicked()
{
	if (DestinationDirectory.IsEmpty())
	{
		DestinationDirectory = FPaths::Combine(FPaths::ProjectContentDir(), "Reactions"); // Default to Content directory /Reactions if none selected
	}

	// Ensure full path is in Content directory
	if (!DestinationDirectory.StartsWith(FPaths::ProjectContentDir()))
	{
		// Show error message
		const TSharedRef<SWindow> ErrorWindow = SNew(SWindow)
			.Title(LOCTEXT("ErrorBox", "Error"))
			.SizingRule(ESizingRule::FixedSize)
			.ClientSize(FVector2D(250, 40))
			.SupportsMinimize(false)
			.Content()
			[

				SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
							.Text(FText::FromString("Error: Invalid Directory"))
					]

			];

		TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
		if (RootWindow.IsValid())
		{
			FSlateApplication::Get().AddWindowAsNativeChild(ErrorWindow, RootWindow.ToSharedRef());
		}
		else
		{
			FSlateApplication::Get().AddWindow(ErrorWindow);
		}
		return;
	}

	// Create directory if it doesn't exist
	if (!FPaths::DirectoryExists(DestinationDirectory))
	{
		IFileManager::Get().MakeDirectory(*DestinationDirectory, true);
	}

	// Generate reactions enums
	UEnumFactory* EnumFactory = NewObject<UEnumFactory>();
	EnumFactory->SupportedClass = UUserDefinedEnum::StaticClass();

	FString LongPackageName = FPackageName::FilenameToLongPackageName(DestinationDirectory);

	// Find all subfolders in the model root path
	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> Folders;
	FString ModelPath = UFloraEngineSettings::GetModelRootPath();
	FileManager.FindFiles(Folders, *(ModelPath + "/*"), false, true);

	// Generate or modify enum for each subfolder
	for (FString& Folder : Folders)
	{
		TSharedPtr<FJsonObject> TemplateObj =  GetTemplateJsonObject(FName(Folder)); // Load json

		if (TemplateObj.IsValid())
		{
			GenerateReactionEnums(Folder, LongPackageName, EnumFactory, TemplateObj);
			if (bGenerateInstructionReactions)
			{
				GenerateInstructionReactions(Folder, FPaths::Combine(LongPackageName, Folder + TEXT("_instructions")), EnumFactory, TemplateObj);
			}
		}
	}
	// Close the window after generation
	FSlateApplication::Get().GetActiveTopLevelWindow()->RequestDestroyWindow();

}

void FFloraEngineSettingsCustomization::GenerateReactionEnums(FString EnumName, FString DestinationPath, UEnumFactory* EnumFactory, TSharedPtr<FJsonObject> TemplateJson)
{
	// Get Enum
	EnumName = EnumName + "_reactions";
	UUserDefinedEnum* Enum = FindOrCreateEnum(EnumName, DestinationPath, EnumFactory);

	TSharedPtr<FJsonObject> ModelOutputs;
	if (TemplateJson->HasField(TEXT("all_combinations"))) { // Template V1
		ModelOutputs = TemplateJson->GetObjectField(TEXT("all_combinations"))->GetObjectField(TEXT("model_output"))->GetObjectField(TEXT("model_results"));
	}
	else if (TemplateJson->HasField(TEXT("tokens"))) { // Template V2
		ModelOutputs = TemplateJson->GetObjectField(TEXT("tokens"))->GetObjectField(TEXT("output"));
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Could not process template in json file for %s"), *EnumName);
		return;
	}

	// Update enum entries
	int EnumCount = Enum->NumEnums() - 1; // Exclude _MAX enumerator
	if (EnumCount > ModelOutputs->Values.Num()) // Remove any excess enumerators
	{
		for (int i = 0; i < EnumCount - ModelOutputs->Values.Num(); i++)
		{
			FEnumEditorUtils::RemoveEnumeratorFromUserDefinedEnum(Enum, 0);
		}
	}

	int index = 0;
	for (auto CurrJsonValue = ModelOutputs->Values.CreateConstIterator(); CurrJsonValue; ++CurrJsonValue)
	{
		if (index >= EnumCount) {
			FEnumEditorUtils::AddNewEnumeratorForUserDefinedEnum(Enum); // Add new enumerator if needed
		}
		FEnumEditorUtils::SetEnumeratorDisplayName(Enum, index++, FText::FromString(CurrJsonValue->Key));
	}

	Enum->MarkPackageDirty();
}

void FFloraEngineSettingsCustomization::GenerateInstructionReactions(FString ModelName, FString DestinationPath, UEnumFactory* EnumFactory, TSharedPtr<FJsonObject> TemplateJson)
{
	if (!TemplateJson->HasField(TEXT("instructions"))) {
		UE_LOG(LogTemp, Warning, TEXT("No instructions found in template for instruction reactions."));
		return;
	}
	TSharedPtr<FJsonObject> ModelOutputs = TemplateJson->GetObjectField(TEXT("tokens"))->GetObjectField(TEXT("output"));

	// Create decrypt map to generate enums based on unencrypted reaction names
	TMap<FString, FString> OutputDecryptMap;
	for (auto CurrJsonValue = ModelOutputs->Values.CreateConstIterator(); CurrJsonValue; ++CurrJsonValue)
	{
		OutputDecryptMap.Add(CurrJsonValue->Value->AsString(), CurrJsonValue->Key);
	}
	// For each instruction, create or modify enum
	TArray<FString> AllInstructions;
	TSharedPtr<FJsonObject> InstructionsObj = TemplateJson->GetObjectField(TEXT("instructions"));
	for (auto CurrInstruction = InstructionsObj->Values.CreateConstIterator(); CurrInstruction; ++CurrInstruction)
	{
		FString InstructionName = CurrInstruction->Key;
		AllInstructions.Add(InstructionName);
		FString EnumName = InstructionName + "_reactions";

		// Find or create enum for this instruction
		UUserDefinedEnum* Enum = FindOrCreateEnum(EnumName, DestinationPath, EnumFactory);

		TArray<TSharedPtr<FJsonValue>> InstructionResults = InstructionsObj->GetObjectField(InstructionName)->GetArrayField(TEXT("output"));

		// Update enum entries
		int EnumCount = Enum->NumEnums() - 1; // Exclude _MAX enumerator
		if (EnumCount > InstructionResults.Num()) // Remove any excess enumerators
		{
			for (int i = 0; i < EnumCount - InstructionResults.Num(); i++)
			{
				FEnumEditorUtils::RemoveEnumeratorFromUserDefinedEnum(Enum, 0);
			}
		}
		
		for (int i = 0; i < InstructionResults.Num(); i++)
		{
			TArray<FString> Lines;
			InstructionResults[i]->AsString().ParseIntoArrayLines(Lines);
			// Second line is the reaction
			if (i >= EnumCount) {
				FEnumEditorUtils::AddNewEnumeratorForUserDefinedEnum(Enum); // Add new enumerator if needed
			}
			FEnumEditorUtils::SetEnumeratorDisplayName(Enum, i, FText::FromString(*OutputDecryptMap.Find(Lines[1]))); // Set to decrypted reaction name
		}
		Enum->MarkPackageDirty();
	}

	// Make enum of all instructions
	FString InstructionsEnumName = ModelName + "_instructions";
	UUserDefinedEnum* InstructionsEnum = FindOrCreateEnum(InstructionsEnumName, DestinationPath, EnumFactory);
	int InstructionsEnumCount = InstructionsEnum->NumEnums() - 1; // Exclude _MAX enumerator
	if (InstructionsEnumCount > AllInstructions.Num()) // Remove any excess enumerators
	{
		for (int i = 0; i < InstructionsEnumCount - AllInstructions.Num(); i++)
		{
			FEnumEditorUtils::RemoveEnumeratorFromUserDefinedEnum(InstructionsEnum, 0);
		}
	}
	for (int i = 0; i < AllInstructions.Num(); i++)
	{
		if (i >= InstructionsEnumCount) {
			FEnumEditorUtils::AddNewEnumeratorForUserDefinedEnum(InstructionsEnum); // Add new enumerator if needed
		}
		FEnumEditorUtils::SetEnumeratorDisplayName(InstructionsEnum, i, FText::FromString(AllInstructions[i]));
	}
	InstructionsEnum->MarkPackageDirty();
}

TSharedPtr<FJsonObject> FFloraEngineSettingsCustomization::GetTemplateJsonObject(FName ModelName)
{
	FString ModelPath = UFloraEngineSettings::GetModelRootPath();
	// Json must have same name as subfolder
	FString JsonPath = UFloraEngineSettings::GetTemplatePath(ModelName);
	if (!FPaths::FileExists(JsonPath)) {
		UE_LOG(LogTemp, Error, TEXT("Could not find json file for model: %s"), *JsonPath);
		return nullptr;
	}
	// Read rections from json
	FString FileText;
	FFileHelper::LoadFileToString(FileText, *JsonPath);

	// Parse json for model results
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(FileText);
	TSharedPtr<FJsonObject> TemplateObj;
	FJsonSerializer::Deserialize(JsonReader, TemplateObj);
	return TemplateObj;
}

UUserDefinedEnum* FFloraEngineSettingsCustomization::FindOrCreateEnum(FString EnumName, FString DestinationPath, UEnumFactory* EnumFactory)
{
	// Check if enum already exists
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	FSoftObjectPath EnumPath = FSoftObjectPath(FPaths::Combine(DestinationPath, EnumName) + "." + EnumName);
	FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(EnumPath);
	UUserDefinedEnum* Enum = nullptr;
	bool bModifyingEnum = false;
	if (AssetData.IsValid()) {
		// Enum already exists, modify it instead of creating new
		Enum = Cast<UUserDefinedEnum>(AssetData.GetAsset());
		if (IsValid(Enum)) {
			bModifyingEnum = true;
			Enum->Modify();

			// Close any open editors for this enum before modifying
			UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
			if (AssetEditorSubsystem)
			{
				TArray<UObject*> Assets;
				Assets.Add(Enum);
				AssetEditorSubsystem->CloseAllEditorsForAsset(Enum);
			}
		}
	}
	if (!bModifyingEnum) // If asset was not an enum then create new
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UObject* NewObj = AssetTools.CreateAsset(EnumName, DestinationPath, UUserDefinedEnum::StaticClass(), EnumFactory);
		Enum = Cast<UUserDefinedEnum>(NewObj);
	}
	return Enum;
}
#undef LOCTEXT_NAMESPACE
