// Copyright © 2025, Databiomes Inc. All rights reserved

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
	void PostInitProperties() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// General settings
	UPROPERTY(config, EditAnywhere, Category = "General", meta = (Tooltip = "The guardrail reaction to ignore when conversing with model."))
	FString GuardrailedReaction = "";

	UPROPERTY(config, EditAnywhere, Category = "General", meta = (Tooltip = "Where the model folders are located within the plugin. Model folders must contain tokenizer.bin, weights.bin and a .json file with the same name as the model folder."))
	FDirectoryPath ModelRootPath;

	UPROPERTY(EditAnywhere, Category = "General")
	FString GenerateReactions;
	
	// Logging settings
	UPROPERTY(config, EditAnywhere, Category = "Logging", meta = (Tooltip = "Creates a log in Saved/Logs showing the inputs and output for models"))
	bool bLoggerActive = false;

	UPROPERTY(config, EditAnywhere, Category = "Logging", meta = (EditCondition = "bLoggerActive", EditConditionHides))
	bool bLogTokenSpeed = false;

	UPROPERTY(config, EditAnywhere, Category = "Logging", meta = (EditCondition = "bLoggerActive", EditConditionHides))
	bool bAlsoLogToUnreal = false;

	UPROPERTY(config, EditAnywhere, Category = "Logging", meta = (EditCondition = "bLoggerActive", EditConditionHides))
	bool bDecryptLog = false;

	// Whisper Settings
	UPROPERTY(VisibleAnywhere, Transient, Category = "Whisper")
	bool bAddonPluginEnabled = false;
	UPROPERTY(config, EditAnywhere, Category = "Whisper", meta = (EditCondition = "bAddonPluginEnabled", FilePathFilter = "Whisper Model (*.bin)|*.bin", Tooltip = "Model to use for Whisper"))
	FFilePath WhisperModel;

	/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Realtime Whisper ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	UPROPERTY(config, EditAnywhere, Category = "Whisper", meta = (Tooltip = "Temporary file name for realtime audio capture"))
	FString RealtimeFileName = "Realtime_Temp";

	UPROPERTY(config, EditAnywhere, Category = "Whisper", meta = (Tooltip = "Temporary file path for realtime audio capture"))
	FString RealtimeFilePath = "";
		~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Realtime Whisper ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	
	UFUNCTION(BlueprintCallable, Category = "FloraEngine|Settings")
	static bool IsLoggerActive();
	UFUNCTION(BlueprintCallable, Category = "FloraEngine|Settings")
	static bool IsTokenSpeedLogged();
	UFUNCTION(BlueprintCallable, Category = "FloraEngine|Settings")
	static bool AlsoLoggingToUnreal();
	UFUNCTION(BlueprintCallable, Category = "FloraEngine|Settings")
	static bool IsDecryptLog();

	UFUNCTION()
	static FString GetModelRootPath();
	UFUNCTION()
	static FString GetTemplatePath(FName ModelName);
	UFUNCTION()
	static FString GetGuardrailedReaction();
	UFUNCTION()
	static FString GetWhisperModelPath();

	/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Realtime Whisper ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	UFUNCTION()
	static FString GetRealtimeFileName();
	UFUNCTION()
	static FString GetRealtimeFilePath();
	   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Realtime Whisper ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
};
