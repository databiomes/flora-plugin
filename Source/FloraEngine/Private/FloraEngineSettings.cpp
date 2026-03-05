// Copyright © 2025, Databiomes Inc. All rights reserved

#include "FloraEngineSettings.h"
#include "Interfaces/IPluginManager.h"
#if WITH_EDITOR
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "FloraEngine.h"
#endif

DEFINE_LOG_CATEGORY(LogFlora);

UFloraEngineSettings::UFloraEngineSettings(const FObjectInitializer& ObjectInitializer) {
	LoadConfig();

	ModelRootPath.Path = TEXT("Content/Models");
}

#if WITH_EDITOR
void UFloraEngineSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	static const FName ModelRootPicker = GET_MEMBER_NAME_CHECKED(UFloraEngineSettings, ModelRootPath);
	static const FName DeviceTypePicker = GET_MEMBER_NAME_CHECKED(UFloraEngineSettings, DeviceType);

	if (!PropertyChangedEvent.Property)
		return;

	if (PropertyChangedEvent.MemberProperty->GetFName() == ModelRootPicker)
	{
		FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("FloraEngine"))->GetBaseDir();
		PluginDir = FPaths::Combine(PluginDir, TEXT("")); // ensures trailing slash
		FPaths::MakePathRelativeTo(ModelRootPath.Path, *PluginDir);

		if (!FPaths::IsRelative(ModelRootPath.Path)) {
			UE_LOG(LogFlora, Warning, TEXT("Model root path must be relative to the Flora Engine plugin directory!"));
		}
	} 
	else if (PropertyChangedEvent.MemberProperty->GetFName() == DeviceTypePicker)
	{
		if (Notification.IsValid())
			return;
		FNotificationInfo Info(FText::FromString("Editor restart required"));
		Info.ButtonDetails.Add(
			FNotificationButtonInfo(
				FText::FromString("Restart Now"),
				FText::FromString("Restart the editor"),
				FSimpleDelegate::CreateLambda([]()
					{
						FUnrealEdMisc::Get().RestartEditor(false);
					})
			)
		);
		Info.bFireAndForget = false;          
		Info.bUseLargeFont = false;
		Info.bUseSuccessFailIcons = false;
		Info.ExpireDuration = 0.0f;           
		Info.FadeOutDuration = 0.5f;
		Info.bAllowThrottleWhenFrameRateIsLow = false;
		Info.bUseThrobber = false;
		Info.Image = FAppStyle::GetBrush("Icons.Warning");

		Notification = FSlateNotificationManager::Get().AddNotification(Info);

		if (Notification.IsValid())
		{
			Notification->SetCompletionState(SNotificationItem::CS_Pending);
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

EDeviceType UFloraEngineSettings::GetDeviceType() 
{
	return GetMutableDefault<UFloraEngineSettings>()->DeviceType;
}
