// Copyright © 2025, Databiomes Inc. All rights reserved

#include "FloraEngine.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "FloraEngineSettings.h"

#define LOCTEXT_NAMESPACE "FFloraEngineModule"

void FFloraEngineModule::StartupModule()
{
	FString BaseDir = IPluginManager::Get().FindPlugin(FLORA_PLUGIN_NAME)->GetBaseDir();
	FString LibraryPath;

	// Determine platform and set library path accordingly
#if PLATFORM_WINDOWS
	FString DeviceType = UEnum::GetValueAsName(UFloraEngineSettings::GetDeviceType()).ToString();
	DeviceType.RemoveFromStart("EDeviceType::");
	if (DeviceType == "intel_cpu") 
		DeviceType = "cpu";
	if (DeviceType == "intel_npu")
		OpenVinoDLLHandle = FPlatformProcess::GetDllHandle(*FPaths::Combine(*BaseDir, "Binaries", "Win64", DeviceType, "openvino.dll"));

	LibraryPath = FPaths::Combine(*BaseDir, "Binaries", "Win64", DeviceType, "flora.dll");
#elif PLATFORM_MAC
	LibraryPath = FPaths::Combine(*BaseDir, FLORA_DYLIB_PATH);
#elif PLATFORM_LINUX || PLATFORM_ANDROID
	LibraryPath = FLORA_SO_PATH;
#endif

#if PLATFORM_WINDOWS // may need for mac and linux
	LibraryPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*LibraryPath);
#endif

	FloraDLLHandle = FPlatformProcess::GetDllHandle(*LibraryPath);

	if (!FloraDLLHandle)
	{
		UE_LOG(LogFlora, Error, TEXT("Failed to load Flora library from path: %s"), *LibraryPath);
		return;
	}
}

void FFloraEngineModule::ShutdownModule()
{
	FreeFloraDLLHandle();
}

void FFloraEngineModule::FreeFloraDLLHandle() 
{
	if (FloraDLLHandle)
	{
		FPlatformProcess::FreeDllHandle(FloraDLLHandle);
	}
	if (OpenVinoDLLHandle)
	{
		FPlatformProcess::FreeDllHandle(OpenVinoDLLHandle);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFloraEngineModule, FloraEngine)