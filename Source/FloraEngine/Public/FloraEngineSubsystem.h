// Copyright © 2026, Databiomes Inc. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h" // Engine
#include "FloraEngineSubsystem.generated.h"

struct NLM {

};

struct NLMOutput {
	float TokenSpeed;
	int Size;
	unsigned char* Buffer;
};

USTRUCT()
struct FModelAsyncPrompt
{
	GENERATED_BODY()

	UPROPERTY()
	UObject* WorldContextObject = nullptr;

	UPROPERTY()
	UNLM* Model = nullptr;

	// Actual prompt text to send to the model
	FString Prompt;

	// Instruction line to prompt with (from template)
	FName InstructionLine;

	// To handle PIE ending while async task is running
	bool bExiting = false;
	FDelegateHandle EndPIEHandle;
};

UCLASS(BlueprintType)
class UNLMOutput : public UObject {
	GENERATED_BODY()

public:
	NLMOutput* Output = nullptr;
};

UCLASS(BlueprintType)
class UNLM : public UObject {
	GENERATED_BODY()

public:
	NLM* Model = nullptr;

	// Number of input lines required for the model (from json file)
	UPROPERTY(BlueprintReadOnly, Category = "FloraEngine")
	int NumInputs = 0;

	// Stores input lines before the prompt, should contain NumInputs - 1 lines
	UPROPERTY(BlueprintReadWrite, Category = "FloraEngine")
	TArray<FString> InputLines;

	// Name of the model set using the custom drop down pin (same as folder name in Content/Models directory)
	FName ModelName;

	// Stores the last instruction line selected
	FString LastInstructionLine;

	// Stores the "all_combinations"->"input_format" json object for the model for use when formatting input
	// Stores the "instructions" object 
	TSharedPtr<FJsonObject> InputFormat;

	// Stores the reverse of the results mapping for encryption/decryption
	TMap<FString, FString> OutputDecryptMap;

	// Store all possible reactions and instruction-reaction mappings for the model
	TMap<FString, uint8> AllReactions;
	TMap<FString, uint8> InstructionIndexMap;
	TMap<FString, TArray<FString>> InstructionReactionMap;
};

#if PLATFORM_ANDROID
#define __stdcall
#endif

typedef NLM* (__stdcall* InitNLMFunc)(const char*, bool);
typedef NLMOutput* (__stdcall* OutputNLMFunc)(NLM*);
typedef void(__stdcall* DeleteNLMFunc)(NLM*);
typedef void(__stdcall* InferenceNLMFunc)(NLM*, const char*);
typedef int(__stdcall* LogAddFilePointerFunc)();

/*
 *	Flora Subsystem Class.
 *	Flora subsystem used to initialize the Flora Engine, create and manage NLM models, and run inference on them.
 */
UCLASS()
class FLORAENGINE_API UFloraEngineSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	// Initializes Flora Engine. Must be called before using the subsystem.
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Initialize Flora Lib", Keywords = "NLM initialize flora", ReturnDisplayName = "Model", WorldContext = "WorldContextObject"), Category = "FloraEngine")
	void Init();

	// Initializes an NLM model from the model directory. Note: Parallel NLM models are currently not supported. NLM must be deleted before initializing a new one
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Initialize NLM Using Model", Keywords = "NLM initialize flora nlm", ReturnDisplayName = "Model", WorldContext = "WorldContextObject"), Category = "FloraEngine")
	UNLM* InitNLM(UObject* WorldContextObject, FName ModelName, TArray<FString> InputLines);

	// Deletes an NLM model. Note: Must be done BEFORE initializing a new model
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Delete NLM", Keywords = "NLM delete flora nlm", WorldContext = "WorldContextObject"), Category = "FloraEngine")
	void DeleteNLM(UNLM* Model);

	// Runs inference on a separate thread and calls the provided function on the game thread when complete
	void AsyncInfer(FModelAsyncPrompt* ModelPrompt, const TFunction<void()>& Fn);

	// Gets the output and reaction from a model after inference. Index of the given instruction and output reaction index are also returned.
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get NLM Output", Keywords = "NLM parse output flora", WorldContext = "WorldContextObject"), Category = "FloraEngine")
	void GetOutput(UObject* WorldContextObject, UNLM* Model, FString& Output, uint8& InstructionIndex, uint8& ReactionIndex, FString& Reaction, float& TokenSpeed);


private:
	// Helper function to parse JSON file for model input format
	void SetupModelFromJson(UObject* WorldContextObject, UNLM* Model);

	// Helper function to get output from NLM model
	UNLMOutput* FloraEngineOutputNLM(UNLM* Model);

	bool bInitialized = false;
	FString ModelRootPath = "";

	InitNLMFunc _DLLInitNLM = nullptr;
	DeleteNLMFunc _DLLDelNLM = nullptr;
	InferenceNLMFunc _DLLInferNLM = nullptr;
	OutputNLMFunc _DLLOutputNLM = nullptr;
	LogAddFilePointerFunc _DLLLogAddFilePointer = nullptr;

	FDelegateHandle EndPIEHandle;
	TArray<FModelAsyncPrompt*> ActivePrompts;

	TArray<UNLM*> NLMs;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncModelOutputPins);

UCLASS()
class UAsyncFloraEngineBPLibrary : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	// Constructor for the AsyncInfer call. Model name is used to load correct input line information
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "AsyncInfer Constructor"), Category = "FloraEngine")
	static UAsyncFloraEngineBPLibrary* AsyncFloraEngineBPLibraryInstructionLine(UObject* WorldContextObject, UNLM* Model, FName ModelName, FName InstructionLine, FString Prompt);

	UPROPERTY(BlueprintAssignable, Category = "FloraEngine")
	FAsyncModelOutputPins OnCompleted;

private:
	virtual void Activate() override;

	FModelAsyncPrompt ModelPrompt;
};





