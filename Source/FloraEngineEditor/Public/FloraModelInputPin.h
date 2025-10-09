// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SGraphPin.h"

/*
 * Flora Model Input Pin class.
 * Custom pin to select input line options from the model's json file.
 */
class FLORAENGINEEDITOR_API SFloraModelInputPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SFloraModelInputPin) {}
		SLATE_ARGUMENT(int, Index)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj);

	// Create the default value widget (override for customized pins)
	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;

	// Handle selection change
	void OnLineSelected(TSharedPtr<FString> ItemSelected, ESelectInfo::Type SelectInfo);
private:
	TArray<TSharedPtr<FString>> LineList;
	TSharedPtr<FString> GetSelectedModelLine();
	void SetPropertyWithString(FString String);
	int JsonIndex = 0;
};
