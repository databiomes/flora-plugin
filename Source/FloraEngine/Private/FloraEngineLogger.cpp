// Copyright © 2025, Databiomes Inc. All rights reserved

#include "FloraEngineLogger.h"
#include "FloraEngineSettings.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"

void UFloraEngineLogger::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Format: FloraEngineLog_YYYY.MM.DD_HH.MM.SS.log
	FileName = "FloraEngineLog_"+ FString::FromInt(FDateTime::Now().GetYear()) + "." +
		FString::FromInt(FDateTime::Now().GetMonth()) + "." +
		FString::FromInt(FDateTime::Now().GetDay()) + "_" +
		FString::FromInt(FDateTime::Now().GetHour()) + "." +
		FString::FromInt(FDateTime::Now().GetMinute()) + "." +
		FString::FromInt(FDateTime::Now().GetSecond()) + ".log";

	LogFilePath = FPaths::ProjectLogDir() + FileName;
}

void UFloraEngineLogger::LogMessage(const FString& Message)
{
	if (UFloraEngineSettings::IsLoggerActive()) {
		FString FinalMessage = Message;
		if (UFloraEngineSettings::IsDecryptLog()) {
			// Decrypt tokens in message
			for (const TPair<FString, FString>& Pair : DecryptMap) {
				FinalMessage = FinalMessage.Replace(*Pair.Key, *Pair.Value);
			}
		}
		FString MessageWithNewLine = FinalMessage + "\n";

		IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

		// Append to file if it exists, otherwise create a new file
		if (FileManager.FileExists(*LogFilePath))
		{
			FFileHelper::SaveStringToFile(MessageWithNewLine, *LogFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
			//UE_LOG(LogTemp, Log, TEXT("Appended to file:%s, %s"), *LogFilePath, *Message);
		}
		else
		{
			FFileHelper::SaveStringToFile(MessageWithNewLine, *LogFilePath, FFileHelper::EEncodingOptions::ForceUTF8);
			//UE_LOG(LogTemp, Log, TEXT("Created new file:%s, %s"), *LogFilePath, *Message);
		}

		if (UFloraEngineSettings::AlsoLoggingToUnreal()) {
			UE_LOG(LogFlora, Log, TEXT("%s"), *FinalMessage);
		}
	}
}

void UFloraEngineLogger::Deinitialize()
{
	Super::Deinitialize();
}