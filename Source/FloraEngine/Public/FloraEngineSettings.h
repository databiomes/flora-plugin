// Copyright © 2025, Databiomes Inc. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#if WITH_EDITOR
#include "Framework/Notifications/NotificationManager.h"
#endif

#include "FloraEngineSettings.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFlora, Log, All);

UENUM(BlueprintType)
enum class EDeviceType : uint8
{
	cpu UMETA(DisplayName = "GeneralCPU"),
	intel_cpu  UMETA(DisplayName = "IntelCPU"),
	intel_npu UMETA(DisplayName = "IntelNPU"),
	amd_npu  UMETA(DisplayName = "AMDNPU")
};

// Configure Flora Engine plugin.
UCLASS(config = Engine, defaultconfig, meta = (DisplayName = "Flora Engine"))
class FLORAENGINE_API UFloraEngineSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
	UFloraEngineSettings(const FObjectInitializer& ObjectInitializer);

	virtual FName GetContainerName() const override { return FName("Project"); }
	virtual FName GetCategoryName() const override { return FName("Plugins"); }
	virtual FName GetSectionName() const override { return FName("Flora Engine"); }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// General settings
	UPROPERTY(config, EditAnywhere, Category = "General", meta = (Tooltip = "Where the model folders are located within the plugin. Model folders must contain tokenizer.bin, weights.bin and a .json file with the same name as the model folder."))
	FDirectoryPath ModelRootPath;

	UPROPERTY(EditAnywhere, Category = "General")
	FString GenerateReactions;

	UPROPERTY(config, EditAnywhere, Category = "General", meta = (Tooltip = "Device that Flora will use."))
	EDeviceType DeviceType = EDeviceType::cpu;
	
	// Logging settings
	UPROPERTY(config, EditAnywhere, Category = "Logging", meta = (Tooltip = "Creates a log in Saved/Logs showing the inputs and output for models"))
	bool bLoggerActive = false;

	UPROPERTY(config, EditAnywhere, Category = "Logging", meta = (EditCondition = "bLoggerActive", EditConditionHides))
	bool bLogTokenSpeed = false;

	UPROPERTY(config, EditAnywhere, Category = "Logging", meta = (EditCondition = "bLoggerActive", EditConditionHides))
	bool bAlsoLogToUnreal = false;

	UPROPERTY(config, EditAnywhere, Category = "Logging", meta = (EditCondition = "bLoggerActive", EditConditionHides))
	bool bDecryptLog = false;
	
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
	static EDeviceType GetDeviceType();

private:
#if WITH_EDITOR
	TSharedPtr<SNotificationItem> Notification;
#endif
};
