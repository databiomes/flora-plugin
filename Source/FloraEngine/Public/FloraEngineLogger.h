// Copyright © 2026, Databiomes Inc. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FloraEngineLogger.generated.h"

/*
 *	Flora Logger class.
 *	Logger subsystem to log model inputs and outputs to a file in Saved/Logs
 */
UCLASS()
class FLORAENGINE_API UFloraEngineLogger : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void AddDecryptToken(const FString& EncryptedToken, const FString& DecryptedToken) {
		DecryptMap.Add(EncryptedToken, DecryptedToken);
	}

	// Log a message to the Flora log file
	UFUNCTION(BlueprintCallable, Category = "FloraEngine|Logger")
	void LogMessage(const FString& Message);
	
private:
	FString LogFilePath = "";
	FString FileName = "";

	// Map to decrypt logged tokens
	TMap<FString, FString> DecryptMap;	
};
