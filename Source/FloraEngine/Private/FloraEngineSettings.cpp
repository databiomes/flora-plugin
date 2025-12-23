// Copyright © 2025, Databiomes Inc. All rights reserved

#include "FloraEngineSettings.h"
#include "Interfaces/IPluginManager.h"

UFloraEngineSettings::UFloraEngineSettings(const FObjectInitializer& ObjectInitializer) {
	LoadConfig();

	ModelRootPath.Path = TEXT("Content/Models");
}

#if WITH_EDITOR
void UFloraEngineSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	static const FName ModelRootPicker = GET_MEMBER_NAME_CHECKED(UFloraEngineSettings, ModelRootPath);

	if (!PropertyChangedEvent.Property)
		return;

	if (PropertyChangedEvent.MemberProperty->GetFName() == ModelRootPicker)
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
