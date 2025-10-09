// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h" // Engine
#include "FloraEngineBPLibrary.generated.h"

/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/

struct NLM {

};

struct NLMOutput {
	float token_speed;
	int size;
	unsigned char* buffer;
};

// Struct to hold indices in the model's json for input line structure
USTRUCT(BlueprintType)
struct FModelInputCombination {
	GENERATED_USTRUCT_BODY()

	FModelInputCombination(int FirstLinePrefix = 0, int SecondLinePrefix = 0, int PromptPrefix = 0) { FirstLinePrefixIndex = FirstLinePrefix, SecondLinePrefixIndex = SecondLinePrefix, PromptPrefixIndex = PromptPrefix;}
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FloraEngine")
	int FirstLinePrefixIndex = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FloraEngine")
	int SecondLinePrefixIndex = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FloraEngine")
	int PromptPrefixIndex = 0;
};

/* 
*	Struct to hold a single input line option for the model's json input structure along with the index of the line in the json file (maybe move to class for the input pin).
*	Will be replaced with a drop down pin in blueprints to select the line from the model's json file.
*/
USTRUCT(BlueprintType)
struct FModelInputLine
{
	GENERATED_USTRUCT_BODY()
public:
	// The actual text of the line as it appears in the json file
	UPROPERTY(BlueprintReadOnly, Category = "FloraEngine")
	FName ModelLine;

	UPROPERTY(BlueprintReadOnly, Category = "FloraEngine")
	int LineIndex = 0;
};

UCLASS(BlueprintType)
class UNLMOutput : public UObject {
	GENERATED_BODY()

public:
	NLMOutput* output = nullptr;
};

UCLASS(BlueprintType)
class UNLM : public UObject {
	GENERATED_BODY()

public:
	NLM* model = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "FloraEngine")
	FString firstLine = "";

	UPROPERTY(BlueprintReadWrite, Category = "FloraEngine")
	FString secondLine = "";

	// Store previous first line in case of guardrail
	FString prevFirstLine = "";

	// Name of the model set using the custom drop down pin (same as folder name in Content/Models directory)
	FName ModelName;

	// Stores the "all_combinations"->"input_format" json object for the model for use when formatting input
	TSharedPtr<FJsonObject> inputFormat;

	// Passes the first and second lines between models. 
	UFUNCTION(BlueprintCallable, Category = "FloraEngine")
	void PassConversation(UNLM* FromModel) {
		firstLine = FromModel->firstLine;
		secondLine = FromModel->secondLine;
	};

};
#if PLATFORM_ANDROID
#define __stdcall
#endif

typedef NLM*(__stdcall* InitNLMFunc)(const char*);
typedef NLMOutput*(__stdcall* OutputNLMFunc)(NLM*);
typedef void(__stdcall* DeleteNLMFunc)(NLM*);
typedef void(__stdcall* InferenceNLMFunc)(NLM*, const char*);
typedef int(__stdcall* LogAddFilePointerFunc)();

UCLASS()
class UFloraEngineBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Initialize Flora Lib", Keywords = "NLM initialize flora", ReturnDisplayName = "Model", WorldContext = "WorldContextObject"), Category = "FloraEngine")
	static void FloraEngineInit();

	// Initialize an instance of a particular model
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Initialize NLM Using Model", Keywords = "NLM initialize flora nlm", ReturnDisplayName = "Model", WorldContext = "WorldContextObject"), Category = "FloraEngine")
	static UNLM* FloraEngineInitNLM(UObject* WorldContextObject, FName ModelName, FString FirstLine, FString SecondLine);

	// Deprecated
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Delete NLM", Keywords = "NLM delete flora"), Category = "FloraEngine")
	static void FloraEngineDeleteNLM(UNLM *Model);

	// Deprecated - use the async version instead
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Infer", Keywords = "NLM infer flora"), Category = "FloraEngine")
	static void FloraEngineInferNLM(UNLM * Model, FString Prompt);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Output", Keywords = "NLM output flora", ReturnDisplayName = "Model Output"), Category = "FloraEngine")
	static UNLMOutput* FloraEngineOutputNLM(UNLM* Model);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Parse Output for Text", Keywords = "NLM parse output flora", WorldContext = "WorldContextObject"), Category = "FloraEngine")
	static void FloraEngineParseOutput(UObject* WorldContextObject, UNLM* Model, UNLMOutput* ModelOutput, FString& output, FString& reaction, float& token_speed);

	// Deprecated
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Parse Input", Keywords = "NLM parse input flora"), Category = "FloraEngine")
	static FString FloraEngineParseInput(FString prompt_input);

	// Helper function to parse JSON file for model input format
	static TSharedPtr<FJsonObject> GetModelInputFromJson(FString SubFolder);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncModelOutputPins);

UCLASS()
class UAsyncFloraEngineBPLibrary : public UBlueprintAsyncActionBase 
{
	GENERATED_UCLASS_BODY()

public:
	// Constructor for the AsyncInfer call. Model name is used to load correct input line information
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "AsyncInfer Constructor"), Category = "FloraEngine")
	static UAsyncFloraEngineBPLibrary* AsyncFloraEngineBPLibrary(UObject* WorldContextObject, UNLM* model, FName ModelName, FModelInputLine InputLine1, FModelInputLine InputLine2, FModelInputLine InputLine3, FString prompt);

	// Constructor for the AsyncInfer call using direct input indices rather than model name and lines (can be more flexible for blueprints)
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "AsyncInfer Constructor Index Inputs"), Category = "FloraEngine")
	static UAsyncFloraEngineBPLibrary* AsyncFloraEngineBPLibraryIndex(UObject* WorldContextObject, UNLM* model, FModelInputCombination Input, FString prompt);

	UPROPERTY(BlueprintAssignable, Category = "FloraEngine")
	FAsyncModelOutputPins OnCompleted;

private:
	virtual void Activate() override;
	UNLM* Model;
	FString Prompt;
	UObject* WorldContextObject;
	FModelInputCombination Inputs;

	// To handle PIE ending while async task is running
	bool bExiting = false;
	FDelegateHandle EndPIEHandle;
};





