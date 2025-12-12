// Copyright © 2025, Databiomes Inc. All rights reserved

#include "FloraEngineSettings.h"
#include "Interfaces/IPluginManager.h"

UFloraEngineSettings::UFloraEngineSettings(const FObjectInitializer& ObjectInitializer) {
	LoadConfig();

	WhisperModel.FilePath = TEXT("Content/Whisper/ggml-tiny.en.bin");
	ModelRootPath.Path = TEXT("Content/Models");
}
void UFloraEngineSettings::PostInitProperties()
{
	Super::PostInitProperties();
	if (IPluginManager::Get().FindPlugin(TEXT("FloraEngineAddon")).IsValid()) {
		bAddonPluginEnabled = IPluginManager::Get().FindPlugin(TEXT("FloraEngineAddon"))->IsEnabled();
	}
	else {
		bAddonPluginEnabled = false;
	}
}

#if WITH_EDITOR
void UFloraEngineSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	static const FName WhisperModelPicker = GET_MEMBER_NAME_CHECKED(UFloraEngineSettings, WhisperModel);
	static const FName ModelRootPicker = GET_MEMBER_NAME_CHECKED(UFloraEngineSettings, ModelRootPath);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.MemberProperty->GetFName() == WhisperModelPicker)
	{
		TSharedPtr<IPlugin> AddonPlugin = IPluginManager::Get().FindPlugin(TEXT("FloraEngineAddon"));
		if (!AddonPlugin)
			return;
		FString PluginDir = AddonPlugin->GetBaseDir();
		PluginDir = FPaths::Combine(PluginDir, TEXT("")); // ensures trailing slash
		// Automatically convert and save the relative path
		FPaths::MakePathRelativeTo(WhisperModel.FilePath, *PluginDir);

		if (!FPaths::IsRelative(WhisperModel.FilePath)) {
			UE_LOG(LogTemp, Warning, TEXT("Whisper Model path must be relative to the Flora Engine plugin directory!"));
		}
	}
	else if (PropertyChangedEvent.Property && PropertyChangedEvent.MemberProperty->GetFName() == ModelRootPicker)
	{
		FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("FloraEngine"))->GetBaseDir();
		PluginDir = FPaths::Combine(PluginDir, TEXT("")); // ensures trailing slash
		FPaths::MakePathRelativeTo(ModelRootPath.Path, *PluginDir);

		if (!FPaths::IsRelative(ModelRootPath.Path)) {
			UE_LOG(LogTemp, Warning, TEXT("Model root path must be relative to the Flora Engine plugin directory!"));
		}
	}
}
#endif

bool UFloraEngineSettings::IsLoggerActive() {
	return GetMutableDefault<UFloraEngineSettings>()->bLoggerActive;
}

bool UFloraEngineSettings::IsTokenSpeedLogged() {
	return GetMutableDefault<UFloraEngineSettings>()->bLogTokenSpeed;
}

bool UFloraEngineSettings::AlsoLoggingToUnreal() {
	return GetMutableDefault<UFloraEngineSettings>()->bAlsoLogToUnreal;
}

bool UFloraEngineSettings::IsDecryptLog() {
	return GetMutableDefault<UFloraEngineSettings>()->bDecryptLog;
}

FString UFloraEngineSettings::GetModelRootPath() {
	FString BaseDir = IPluginManager::Get().FindPlugin("FloraEngine")->GetBaseDir();
	FString FullPath = FPaths::ConvertRelativePathToFull(*BaseDir, GetMutableDefault<UFloraEngineSettings>()->ModelRootPath.Path);
	return FullPath;
}

FString UFloraEngineSettings::GetTemplatePath(FName ModelName) {
#if PLATFORM_WINDOWS
	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::Combine(*GetModelRootPath(), ModelName.ToString(), *(ModelName.ToString() + "_template.json")));
#elif PLATFORM_ANDROID
	return FPaths::Combine(*GetModelRootPath(), ModelName.ToString(), *(ModelName.ToString() + "_template.json"));
#endif
}

FString UFloraEngineSettings::GetGuardrailedReaction() {
	return GetMutableDefault<UFloraEngineSettings>()->GuardrailedReaction;
}

FString UFloraEngineSettings::GetWhisperModelPath() {
	FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("FloraEngineAddon"))->GetBaseDir();
	FString FullPath = FPaths::ConvertRelativePathToFull(*PluginDir, GetMutableDefault<UFloraEngineSettings>()->WhisperModel.FilePath);
	return FullPath;
}

/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Realtime Whisper ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FString UFloraEngineSettings::GetRealtimeFileName() {
	return GetMutableDefault<UFloraEngineSettings>()->RealtimeFileName;
}

FString UFloraEngineSettings::GetRealtimeFilePath() {
	return GetMutableDefault<UFloraEngineSettings>()->RealtimeFilePath;
}
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Realtime Whisper ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */