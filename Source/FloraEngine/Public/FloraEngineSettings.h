// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FloraEngineSettings.generated.h"

/*
 *	Flora Engine Settings class.
 *	Settings for the Flora Engine plugin. Accessible in Project Settings under Plugins->Flora Engine.
 */
UCLASS(config = Engine, defaultconfig)
class FLORAENGINE_API UFloraEngineSettings : public UObject
{
public:
	GENERATED_BODY()
	UFloraEngineSettings(const FObjectInitializer& ObjectInitializer);

	// General settings
	UPROPERTY(config, EditAnywhere, Category = "General", meta = (Tooltip = "The guardrail reaction to ignore when conversing with model."))
	FString GuardrailedReaction = "";

	UPROPERTY(config, EditAnywhere, Category = "General", meta = (Tooltip = "Where the model folders are located within the plugin. Model folders must contain tokenizer.bin, weights.bin and a .json file with the same name as the model folder."))
	FString ModelRootPath = "Content/Models";
	
	// Logging settings
	UPROPERTY(config, EditAnywhere, Category = "Logging", meta = (Tooltip = "Creates a log in Saved/Logs showing the inputs and output for models"))
	bool bLoggerActive = false;

	UPROPERTY(config, EditAnywhere, Category = "Logging", meta = (EditCondition = "bLoggerActive", EditConditionHides))
	bool bLogTokenSpeed = false;

	UPROPERTY(config, EditAnywhere, Category = "Logging", meta = (EditCondition = "bLoggerActive", EditConditionHides))
	bool bAlsoLogToUnreal = false;


	UFUNCTION(BlueprintCallable, Category = "FloraEngine|Settings")
	static bool IsLoggerActive();
	UFUNCTION(BlueprintCallable, Category = "FloraEngine|Settings")
	static bool IsTokenSpeedLogged();
	UFUNCTION(BlueprintCallable, Category = "FloraEngine|Settings")
	static bool AlsoLoggingToUnreal();
	UFUNCTION()
	static FString GetModelRootPath();
	UFUNCTION()
	static FString GetGuardrailedReaction();

};
