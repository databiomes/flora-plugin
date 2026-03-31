// Copyright © 2026, Databiomes Inc. All rights reserved

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


#include "Kismet2/StructureEditorUtils.h"
#include "UserDefinedStructure/UserDefinedStructEditorData.h"
#include "DataTableEditorUtils.h"

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

		// Hide the original properties
		TSharedPtr<IPropertyHandle> GenerateReactionsProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UFloraEngineSettings, GenerateReactions));
		GenerateReactionsProperty->MarkHiddenByCustomization();

		TSharedPtr<IPropertyHandle> GenerateStatesProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UFloraEngineSettings, GenerateStates));
		GenerateStatesProperty->MarkHiddenByCustomization();

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
							.OnClicked_Lambda([this]() // Open window with directory picker and generate button for reactions enums
								{
									const TSharedRef<SWindow> Window = SNew(SWindow)
										.Title(LOCTEXT("GenerateReactionsTitle", "Generate Reactions"))
										.SizingRule(ESizingRule::Autosized)
										//.ClientSize(FVector2D(700, 125))
										.SupportsMinimize(false)
										.Content()
										[
											SAssignNew(ReactionsVerticalBox, SVerticalBox)
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
																		EnumDestinationDirectory = NewDir;
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

											// Row: Checkbox for model selection
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
																.Text(FText::FromString("Generate All Models: "))
														]
														+ SHorizontalBox::Slot()
														.FillWidth(1.0f)
														.HAlign(HAlign_Right)
														.Padding(10, 0, 10, 0)
														[
															// Checkbox for all models
															SNew(SCheckBox)
																.IsChecked_Lambda([this]() -> ECheckBoxState
																	{
																		return bGenerateAllModelReactions ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
																	})
																.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
																	{
																		bGenerateAllModelReactions = (NewState == ECheckBoxState::Checked);

																		if (!bGenerateAllModelReactions)
																		{
																			UpdateReactionModelCheckboxes();
																		}
																		else
																		{
																			for (TSharedPtr<SWidget> Checkbox : ReactionsModelCheckboxes)
																			{
																				ReactionsVerticalBox->RemoveSlot(Checkbox.ToSharedRef());
																			}
																		}
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

									if (!bGenerateAllModelReactions)
										UpdateReactionModelCheckboxes();
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
			else if (Property->GetProperty() == GenerateStatesProperty->GetProperty()) {
				FloraEngineCategory.AddCustomRow(GenerateStatesProperty->GetPropertyDisplayName(), /*bForAdvanced*/false)
					.NameContent()
					[
						GenerateStatesProperty->CreatePropertyNameWidget()
					]
					.ValueContent()
					[
						SNew(SButton)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.OnClicked_Lambda([this]() // Open window with directory picker and generate button for state struct and data table 
								{
									const TSharedRef<SWindow> Window = SNew(SWindow)
										.Title(LOCTEXT("GenerateStatesTitle", "Generate States"))
										.SizingRule(ESizingRule::Autosized)
										.SupportsMinimize(false)
										.Content()
										[
											SAssignNew(StateVerticalBox, SVerticalBox)
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
																.Text(FText::FromString("State struct and data table destination: "))
														]

														+ SHorizontalBox::Slot()
														.FillWidth(1.0f)
														.Padding(10, 0, 0, 0)
														[
															// Directory picker
															SNew(SDirectoryPicker)
																.Directory(FPaths::Combine(FPaths::ProjectContentDir(), "States")) // Default directory
																.OnDirectoryChanged_Lambda([this](const FString& NewDir)
																	{
																		StateDestinationDirectory = NewDir;
																	})
														]
												]
											// Row: Checkbox for model selection
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
																.Text(FText::FromString("Generate All Models: "))
														]
														+ SHorizontalBox::Slot()
														.FillWidth(1.0f)
														.HAlign(HAlign_Right)
														.Padding(10, 0, 10, 0)
														[
															// Checkbox for all models
															SNew(SCheckBox)
																.IsChecked_Lambda([this]() -> ECheckBoxState
																	{
																		return bGenerateAllModelStates ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
																	})
																.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
																	{
																		bGenerateAllModelStates = (NewState == ECheckBoxState::Checked);

																		if (!bGenerateAllModelStates)
																		{
																			UpdateStateModelCheckboxes();
																		}
																		else
																		{
																			for (TSharedPtr<SWidget> Checkbox : StateModelCheckboxes)
																			{
																				StateVerticalBox->RemoveSlot(Checkbox.ToSharedRef());
																			}
																		}
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
																OnGenerateStatesClicked();
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

									if (!bGenerateAllModelStates)
										UpdateStateModelCheckboxes();

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
											.Text(LOCTEXT("GenerateStates", "Generate States..."))
									]
							]
					];
			}
		}
	}
}

void FFloraEngineSettingsCustomization::UpdateReactionModelCheckboxes()
{
	// Find all subfolders in the model root path
	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> Folders;
	FString ModelPath = UFloraEngineSettings::GetModelRootPath();
	FileManager.FindFiles(Folders, *(ModelPath + "/*"), false, true);
	int Count = 0;
	// Generate or modify enum for each subfolder
	for (FString& Folder : Folders)
	{
		TSharedPtr<FJsonObject> TemplateObj = GetTemplateJsonObject(FName(Folder)); // Load json

		if (TemplateObj.IsValid())
		{
			if (TemplateObj->HasField(TEXT("tokens")))
			{
				TSharedPtr<SHorizontalBox> ModelCheckbox;
				ReactionsVerticalBox->InsertSlot(3 + Count) // Insert after the "Generate All Models" checkbox
					.AutoHeight()
					.Padding(20, 10)
					[
						SAssignNew(ModelCheckbox, SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
									.Text(FText::FromString(Folder))
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.HAlign(HAlign_Right)
							.Padding(10, 0, 10, 0)
							[
								// Checkbox for all models
								SNew(SCheckBox)
									.IsChecked_Lambda([this, Folder]() -> ECheckBoxState
										{
											if (ModelReactionsMap.Contains(Folder)) {
												return ModelReactionsMap[Folder] ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
											}
											else {
												return ECheckBoxState::Unchecked;
											}
										})
									.OnCheckStateChanged_Lambda([this, Folder](ECheckBoxState NewState)
										{
											ModelReactionsMap.FindOrAdd(Folder) = (NewState == ECheckBoxState::Checked);
										})
							]
					];
				ReactionsModelCheckboxes.Add(ModelCheckbox);
				Count++;
			}
		}
	}
}

void FFloraEngineSettingsCustomization::UpdateStateModelCheckboxes()
{
	// Find all subfolders in the model root path
	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> Folders;
	FString ModelPath = UFloraEngineSettings::GetModelRootPath();
	FileManager.FindFiles(Folders, *(ModelPath + "/*"), false, true);
	int Count = 0;
	// Generate or modify enum for each subfolder
	for (FString& Folder : Folders)
	{
		TSharedPtr<FJsonObject> TemplateObj = GetTemplateJsonObject(FName(Folder)); // Load json

		if (TemplateObj.IsValid())
		{
			if (TemplateObj->HasField(TEXT("state_machine")) && TemplateObj->GetBoolField(TEXT("state_machine")))
			{
				TSharedPtr<SHorizontalBox> ModelCheckbox;
				StateVerticalBox->InsertSlot(2 + Count) // Insert after the "Generate All Models" checkbox
					.AutoHeight()
					.Padding(20, 10)
					[
						SAssignNew(ModelCheckbox, SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
									.Text(FText::FromString(Folder))
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.HAlign(HAlign_Right)
							.Padding(10, 0, 10, 0)
							[
								// Checkbox for all models
								SNew(SCheckBox)
									.IsChecked_Lambda([this, Folder]() -> ECheckBoxState
										{
											if (ModelStateMap.Contains(Folder)) {
												return ModelStateMap[Folder] ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
											}
											else {
												return ECheckBoxState::Unchecked;
											}
										})
									.OnCheckStateChanged_Lambda([this, Folder](ECheckBoxState NewState)
										{
											ModelStateMap.FindOrAdd(Folder) = (NewState == ECheckBoxState::Checked);
										})
							]
					];
				StateModelCheckboxes.Add(ModelCheckbox);
				Count++;
			}
		}
	}
}

void FFloraEngineSettingsCustomization::OnGenerateReactionsClicked()
{
	if (EnumDestinationDirectory.IsEmpty())
	{
		EnumDestinationDirectory = FPaths::Combine(FPaths::ProjectContentDir(), "Reactions"); // Default to Content directory /Reactions if none selected
	}

	// Ensure full path is in Content directory
	if (!EnumDestinationDirectory.StartsWith(FPaths::ProjectContentDir()))
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
	if (!FPaths::DirectoryExists(EnumDestinationDirectory))
	{
		IFileManager::Get().MakeDirectory(*EnumDestinationDirectory, true);
	}

	// Generate reactions enums
	UEnumFactory* EnumFactory = NewObject<UEnumFactory>();
	EnumFactory->SupportedClass = UUserDefinedEnum::StaticClass();

	FString LongPackageName = FPackageName::FilenameToLongPackageName(EnumDestinationDirectory);

	// Find all subfolders in the model root path
	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> Folders;
	FString ModelPath = UFloraEngineSettings::GetModelRootPath();
	FileManager.FindFiles(Folders, *(ModelPath + "/*"), false, true);

	if (bGenerateAllModelReactions) {
		// Generate or modify enum for each subfolder
		for (FString& Folder : Folders)
		{
			TSharedPtr<FJsonObject> TemplateObj = GetTemplateJsonObject(FName(Folder)); // Load json

			if (TemplateObj.IsValid())
			{
				GenerateReactionEnums(Folder, LongPackageName, EnumFactory, TemplateObj);
				if (bGenerateInstructionReactions)
				{
					GenerateInstructionReactions(Folder, FPaths::Combine(LongPackageName, Folder + TEXT("_instructions")), EnumFactory, TemplateObj);
				}
			}
		}
	}
	else 
	{
		for (auto& Model : ModelReactionsMap)
		{
			if (Model.Value) // If this model is selected for state generation
			{
				TSharedPtr<FJsonObject> TemplateObj = GetTemplateJsonObject(FName(Model.Key)); // Load json
				GenerateReactionEnums(Model.Key, LongPackageName, EnumFactory, TemplateObj);
				if (bGenerateInstructionReactions)
				{
					GenerateInstructionReactions(Model.Key, FPaths::Combine(LongPackageName, Model.Key + TEXT("_instructions")), EnumFactory, TemplateObj);
				}
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

void FFloraEngineSettingsCustomization::OnGenerateStatesClicked()
{
	if (StateDestinationDirectory.IsEmpty())
	{
		StateDestinationDirectory = FPaths::Combine(FPaths::ProjectContentDir(), "States"); // Default to Content directory /States if none selected
	}

	// Ensure full path is in Content directory
	if (!StateDestinationDirectory.StartsWith(FPaths::ProjectContentDir()))
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
	if (!FPaths::DirectoryExists(StateDestinationDirectory))
	{
		IFileManager::Get().MakeDirectory(*StateDestinationDirectory, true);
	}
	FString LongPackageName = FPackageName::FilenameToLongPackageName(StateDestinationDirectory);

	UStructureFactory* StructFactory = NewObject<UStructureFactory>();
	StructFactory->SupportedClass = UUserDefinedStruct::StaticClass();
	UDataTableFactory* DataTableFactory = NewObject<UDataTableFactory>();

	if (bGenerateAllModelStates) {
		// Find all subfolders in the model root path
		IFileManager& FileManager = IFileManager::Get();
		TArray<FString> Folders;
		FString ModelPath = UFloraEngineSettings::GetModelRootPath();
		FileManager.FindFiles(Folders, *(ModelPath + "/*"), false, true);

		// Generate or modify enum for each subfolder
		for (FString& Folder : Folders)
		{
			TSharedPtr<FJsonObject> TemplateObj = GetTemplateJsonObject(FName(Folder)); // Load json

			if (TemplateObj.IsValid())
			{
				if (!TemplateObj->GetBoolField(TEXT("state_machine"))) {
					UE_LOG(LogTemp, Warning, TEXT("%s is not a state machine model."), *Folder);
					continue;
				}

				UUserDefinedStruct* Struct = GenerateStateStruct("F_" + Folder, LongPackageName, StructFactory);
				GenerateStateDatatable("DT_" + Folder, LongPackageName, DataTableFactory, TemplateObj, Struct);
			}
		}
	}
	else {
		// Generate states for selected models if not generating all
		for (auto& Model : ModelStateMap)
		{
			if (Model.Value) // If this model is selected for state generation
			{
				TSharedPtr<FJsonObject> TemplateObj = GetTemplateJsonObject(FName(Model.Key)); // Load json

				UUserDefinedStruct* Struct = GenerateStateStruct("F_" + Model.Key, LongPackageName, StructFactory);
				GenerateStateDatatable("DT_" + Model.Key, LongPackageName, DataTableFactory, TemplateObj, Struct);
			}
		}
	}
	// Close the window after generation
	FSlateApplication::Get().GetActiveTopLevelWindow()->RequestDestroyWindow();
}

UUserDefinedStruct* FFloraEngineSettingsCustomization::GenerateStateStruct(FString StructName, FString DestinationPath, UStructureFactory* StructFactory)
{
	// Check if struct already exists
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	FSoftObjectPath StructPath = FSoftObjectPath(FPaths::Combine(DestinationPath, StructName) + "." + StructName);
	FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(StructPath);
	UUserDefinedStruct* Struct = nullptr;

	// Struct already exists, do not modify
	if (AssetData.IsValid()) {
		Struct = Cast<UUserDefinedStruct>(AssetData.GetAsset());
		return Struct;
	}

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	UObject* NewObj = AssetTools.CreateAsset(StructName, DestinationPath, UUserDefinedStruct::StaticClass(), StructFactory);
	Struct = Cast<UUserDefinedStruct>(NewObj);

	// Remove default bool member and add new string default
	FEdGraphPinType NewDefaultVariable;
	NewDefaultVariable.PinCategory = UEdGraphSchema_K2::PC_String;
	TArray<FStructVariableDescription>& Vars = FStructureEditorUtils::GetVarDesc(Struct);
	FGuid DefaultBool = Vars[0].VarGuid;

	FStructureEditorUtils::AddVariable(Struct, NewDefaultVariable);
	Vars = FStructureEditorUtils::GetVarDesc(Struct);
	FStructureEditorUtils::RenameVariable(Struct, Vars[1].VarGuid, "Text");
	FStructureEditorUtils::RemoveVariable(Struct, DefaultBool);

	Struct->MarkPackageDirty();
	return Struct;
}

void FFloraEngineSettingsCustomization::GenerateStateDatatable(FString DatatableName, FString DestinationPath, UDataTableFactory* DatatableFactory, TSharedPtr<FJsonObject> TemplateJson, UUserDefinedStruct* Struct)
{
	// Check if datatable already exists
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	FSoftObjectPath DTPath = FSoftObjectPath(FPaths::Combine(DestinationPath, DatatableName) + "." + DatatableName);
	FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(DTPath);
	UDataTable* DataTable = nullptr;
	// Datatable already exists, add new rows if needed
	if (AssetData.IsValid()) {
		DataTable = Cast<UDataTable>(AssetData.GetAsset());
	}
	else {
		// Create new datatable
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		DatatableFactory->Struct = Struct;
		UObject* DTAsset = AssetTools.CreateAsset(
			DatatableName,
			DestinationPath,
			UDataTable::StaticClass(),
			DatatableFactory
		);

		DataTable = Cast<UDataTable>(DTAsset);
		FAssetRegistryModule::AssetCreated(DataTable);
	}

	auto States = TemplateJson->GetArrayField(TEXT("states"));
	for (auto State : States)
	{
		FString StateName = State->AsString();
		// Add row for each state if it doesn't already exist
		FDataTableEditorUtils::AddRow(DataTable, FName(*StateName));
	}

	DataTable->MarkPackageDirty();
}
#undef LOCTEXT_NAMESPACE
