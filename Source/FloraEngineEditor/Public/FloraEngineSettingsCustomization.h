// Copyright © 2025, Databiomes Inc. All rights reserved

#pragma once

#include "IDetailCustomization.h"
#include "Factories/EnumFactory.h"
#include "Engine/UserDefinedEnum.h"

class IDetailLayoutBuilder;
/*
 * Flora Engine Settings Customization class.
 * Customizes the Flora Engine settings in Project Settings to add a button for generating reaction enums.
 * Also has functionality for generating the UserDefinedEnum assets or modifying existing ones.
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

	// The directory where the generated enums will be saved. Set via the directory picker.
	FString DestinationDirectory;

	// Whether to generate instruction specific reactions or not (from template V2)
	bool bGenerateInstructionReactions = false;
};
