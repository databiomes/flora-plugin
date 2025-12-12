// Copyright © 2025, Databiomes Inc. All rights reserved

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
	FString Prompt;

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

	UPROPERTY(BlueprintReadWrite, Category = "FloraEngine")
	FString FirstLine = "";

	UPROPERTY(BlueprintReadWrite, Category = "FloraEngine")
	FString SecondLine = "";

	// Store previous first line in case of guardrail
	FString PrevFirstLine = "";

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

	// Passes the first and second lines between models. 
	UFUNCTION(BlueprintCallable, Category = "FloraEngine")
	void PassConversation(UNLM* FromModel) {
		FirstLine = FromModel->FirstLine;
		SecondLine = FromModel->SecondLine;
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

// Library for c++
UCLASS()
class UFloraEngineLibrary : public UObject
{
	GENERATED_BODY()

public:
	static void Init();

	static UNLM* InitNLM(UObject* WorldContextObject, FName ModelName, FString FirstLine, FString SecondLine);

	static void AsyncInfer(const FModelAsyncPrompt& ModelPrompt, const TFunction<void()>& Fn);

	static void GetOutput(UObject* WorldContextObject, UNLM* Model, FString& Output, FString& Reaction, float& TokenSpeed);

	// Helper function to parse JSON file for model input format
	static void SetupModelFromJson(UObject* WorldContextObject, UNLM* Model);

	// Helper function to get output from NLM model
	static UNLMOutput* FloraEngineOutputNLM(UNLM* Model);
};

// Blueprint library
UCLASS()
class UFloraEngineBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Initialize Flora Lib", Keywords = "NLM initialize flora", ReturnDisplayName = "Model", WorldContext = "WorldContextObject"), Category = "FloraEngine")
	static void FloraEngineInit();

	// Initialize an instance of a particular model
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Initialize NLM Using Model", Keywords = "NLM initialize flora nlm", ReturnDisplayName = "Model", WorldContext = "WorldContextObject"), Category = "FloraEngine")
	static UNLM* FloraEngineInitNLM(UObject* WorldContextObject, FName ModelName, FString FirstLine, FString SecondLine);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get NLM Output", Keywords = "NLM parse output flora", WorldContext = "WorldContextObject"), Category = "FloraEngine")
	static void FloraEngineGetOutput(UObject* WorldContextObject, UNLM* Model, FString& Output, uint8& ReactionIndex, FString& Reaction, float& TokenSpeed);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get NLM Output With Instruction", Keywords = "NLM parse output flora", WorldContext = "WorldContextObject"), Category = "FloraEngine")
	static void FloraEngineGetOutputWithInstruction(UObject* WorldContextObject, UNLM* Model, FString& Output, uint8& InstructionIndex, uint8& ReactionIndex, FString& Reaction, float& TokenSpeed);
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





