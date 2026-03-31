// Copyright © 2026, Databiomes Inc. All rights reserved

#pragma once

#include "IDetailCustomization.h"
#include "Factories/EnumFactory.h"
#include "Engine/UserDefinedEnum.h"
#include "StructUtils/UserDefinedStruct.h"
#include "Factories/StructureFactory.h"
#include "Factories/DataTableFactory.h"
#include "Engine/DataTable.h"

class IDetailLayoutBuilder;
/*
 * Flora Engine Settings Customization class.
 * Customizes the Flora Engine settings in Project Settings to add a button for generating reaction enums and state machine structures and datatables.
 */
class FLORAENGINEEDITOR_API FFloraEngineSettingsCustomization : public IDetailCustomization
{
public:
	FFloraEngineSettingsCustomization();
	virtual ~FFloraEngineSettingsCustomization() override;

	static TSharedRef<IDetailCustomization> MakeInstance();

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
	// End of IDetailCustomization interface

private:
	// ~~~~~~~~~~ Reactions ~~~~~~~~~~
	// Helper function for when the Generate Reactions button is clicked
	void OnGenerateReactionsClicked();

	// Generate reaction enums based on the template JSON
	void GenerateReactionEnums(FString EnumName, FString DestinationPath, UEnumFactory* EnumFactory, TSharedPtr<FJsonObject> TemplateJson);

	// Generate instruction reaction enums and instructions enum based on the template JSON
	void GenerateInstructionReactions(FString ModelName, FString DestinationPath, UEnumFactory* EnumFactory, TSharedPtr<FJsonObject> TemplateJson);

	// Find an existing enum by name or create a new one at the specified path
	UUserDefinedEnum* FindOrCreateEnum(FString EnumName, FString DestinationPath, UEnumFactory* EnumFactory);

	// Helper function to load the template JSON object for a given model name
	TSharedPtr<FJsonObject> GetTemplateJsonObject(FName ModelName);

	// Helper function to create slate widgets for each model that can generate reactions
	void UpdateReactionModelCheckboxes();

	// References to the popup window for reactions for dynamic UI
	TSharedPtr<SVerticalBox> ReactionsVerticalBox;
	TArray<TSharedPtr<SHorizontalBox>> ReactionsModelCheckboxes;

	// Whether enums should be generated for all available models
	bool bGenerateAllModelReactions = true;

	// The directory where the generated enums will be saved. Set via the directory picker.
	FString EnumDestinationDirectory;

	// Whether to generate instruction specific reactions or not (from template V2)
	bool bGenerateInstructionReactions = false;

	// Map of model names to whether their reactions should be generated or not, set via checkboxes in the UI
	TMap<FString, bool> ModelReactionsMap;

	// ~~~~~~~~~~ States ~~~~~~~~~~
	// Helper function for when the Generate States button is clicked
	void OnGenerateStatesClicked();

	// Generate struct for model
	UUserDefinedStruct* GenerateStateStruct(FString StructName, FString DestinationPath, UStructureFactory* StructFactory);

	// Generate state datatable based on the template JSON
	void GenerateStateDatatable(FString DatatableName, FString DestinationPath, UDataTableFactory* DatatableFactory, TSharedPtr<FJsonObject> TemplateJson, UUserDefinedStruct* Struct);

	// Helper function to create slate widgets for each model that is a state machine
	void UpdateStateModelCheckboxes();

	// References to the popup window for states for dynamic UI
	TSharedPtr<SVerticalBox> StateVerticalBox;
	TArray<TSharedPtr<SHorizontalBox>> StateModelCheckboxes;

	// Whether datatables and structs should be generated for all available models
	bool bGenerateAllModelStates = true;

	// The directory where the generated state struct and datatable will be saved. Set via the directory picker.
	FString StateDestinationDirectory;

	// Map of model names to whether their states should be generated or not, set via checkboxes in the UI
	TMap<FString, bool> ModelStateMap; 
};
