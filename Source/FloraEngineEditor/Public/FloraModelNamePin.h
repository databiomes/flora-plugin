// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SGraphPin.h"
#include "SNameComboBox.h"

/*
 * Flora Model Name Pin class.
 * Custom pin to select model name from the Models root directory.
 */
class FLORAENGINEEDITOR_API SFloraModelNamePin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SFloraModelNamePin) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj);

	// Create the default value widget (override for customized pins)
	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;

	// Handle selection change
	void OnNameSelected(TSharedPtr<FName> ItemSelected, ESelectInfo::Type SelectInfo);
private:
	TArray<TSharedPtr<FName>> ModelList;
	TSharedPtr<FName>  GetSelectedName();
	void SetPropertyWithName(FName& Name);
	void GetPropertyAsName(FName& OutName);
	void OnComboBoxOpening();
	TSharedPtr<SNameComboBox> NameComboBox;
};
