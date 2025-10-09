// Copyright Epic Games, Inc. All Rights Reserved.


#include "FloraEngineSettings.h"

UFloraEngineSettings::UFloraEngineSettings(const FObjectInitializer& ObjectInitializer) {
	LoadConfig();
}

bool UFloraEngineSettings::IsLoggerActive() {
	return GetMutableDefault<UFloraEngineSettings>()->bLoggerActive;
}

bool UFloraEngineSettings::IsTokenSpeedLogged() {
	return GetMutableDefault<UFloraEngineSettings>()->bLogTokenSpeed;
}

bool UFloraEngineSettings::AlsoLoggingToUnreal() {
	return GetMutableDefault<UFloraEngineSettings>()->bAlsoLogToUnreal;
}

FString UFloraEngineSettings::GetModelRootPath() {
	return GetMutableDefault<UFloraEngineSettings>()->ModelRootPath;
}

FString UFloraEngineSettings::GetGuardrailedReaction() {
	return GetMutableDefault<UFloraEngineSettings>()->GuardrailedReaction;
}