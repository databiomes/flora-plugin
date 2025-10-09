// Copyright Epic Games, Inc. All Rights Reserved.

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

	// Log a message to the Flora log file
	UFUNCTION(BlueprintCallable, Category = "FloraEngine|Logger")
	void LogMessage(const FString& Message);
	
private:
	FString LogFilePath = "";
	FString FileName = "";
};
