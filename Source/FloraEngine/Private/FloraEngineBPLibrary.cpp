// Copyright Epic Games, Inc. All Rights Reserved.

#include "FloraEngineBPLibrary.h"
#include "FloraEngineSettings.h"
#include "FloraEngineLogger.h"

#include "Interfaces/IPluginManager.h"
#include "Kismet/GameplayStatics.h"
#include <string>
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonValue.h"
#include "HAL/PlatformFileManager.h"

static InitNLMFunc _DLLInitNLM = nullptr;
static DeleteNLMFunc _DLLDelNLM = nullptr;
static InferenceNLMFunc _DLLInferNLM = nullptr;
static OutputNLMFunc _DLLOutputNLM = nullptr;
static LogAddFilePointerFunc _DLLLogAddFilePointer = nullptr;

static bool initialized = false;
static FString ModelRootPath = "";

UFloraEngineBPLibrary::UFloraEngineBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	//UE_LOG(LogTemp, Warning, TEXT("HELLO FROM INIT"));
}

FString GetSystemErrorMessage(uint32 ErrorCode)
{
	TCHAR Buffer[256];
	FPlatformMisc::GetSystemErrorMessage(Buffer, 256, ErrorCode);
	return FString(Buffer);
}

void UFloraEngineBPLibrary::FloraEngineInit() {
	if (initialized) return;

	initialized = true;
	void* DLLModuleFlora = nullptr;

	FString BaseDir = IPluginManager::Get().FindPlugin("FloraEngine")->GetBaseDir();
	FString LibraryPath;

	// Determine platform and set library path accordingly
#if PLATFORM_WINDOWS	
	LibraryPath = FPaths::Combine(*BaseDir, FLORA_DLL_PATH);
#elif PLATFORM_MAC
	LibraryPath = FPaths::Combine(*BaseDir, FLORA_DYLIB_PATH);
#elif PLATFORM_LINUX || PLATFORM_ANDROID
	LibraryPath = FLORA_SO_PATH;
#endif
	LibraryPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*LibraryPath);

	ModelRootPath = FPaths::Combine(*BaseDir, UFloraEngineSettings::GetModelRootPath());

#if PLATFORM_WINDOWS // may need for mac and linux
	ModelRootPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*ModelRootPath);
#endif

	DLLModuleFlora = FPlatformProcess::GetDllHandle(*LibraryPath);

	if (!DLLModuleFlora)
	{
		uint32 ErrorCode = FPlatformMisc::GetLastError();
		FString ErrorMessage = GetSystemErrorMessage(ErrorCode);
		UE_LOG(LogTemp, Error, TEXT("Failed to load DLL: %s (Error %d: %s)"), *LibraryPath, ErrorCode, *ErrorMessage);
		UE_LOG(LogTemp, Error, TEXT("ThirdPartyLibraryError"));
		initialized = false;
		return;
	}

	// Get function pointers
	FString funcname = "InitNLM";
	_DLLInitNLM = (InitNLMFunc)FPlatformProcess::GetDllExport(DLLModuleFlora, *funcname);

	funcname = "Delete";
	_DLLDelNLM = (DeleteNLMFunc)FPlatformProcess::GetDllExport(DLLModuleFlora, *funcname);

	funcname = "Inference";
	_DLLInferNLM = (InferenceNLMFunc)FPlatformProcess::GetDllExport(DLLModuleFlora, *funcname);

	funcname = "OutputNLM";
	_DLLOutputNLM = (OutputNLMFunc)FPlatformProcess::GetDllExport(DLLModuleFlora, *funcname);

	funcname = "LogAddFilePointer";
	_DLLLogAddFilePointer = (LogAddFilePointerFunc)FPlatformProcess::GetDllExport(DLLModuleFlora, *funcname);
}

UNLM* UFloraEngineBPLibrary::FloraEngineInitNLM(UObject* WorldContextObject, FName ModelName, FString FirstLine, FString SecondLine)
{
	if (!initialized) {
		UE_LOG(LogTemp, Error, TEXT("Flora not initialized, use the Flora Engine Init node first!"));
		return nullptr;
	}

	UNLM* nlm = NewObject<UNLM>((UObject*)GetTransientPackage(), UNLM::StaticClass());

	// Set path using the model name
	FString Path;
	Path = FPaths::Combine(*ModelRootPath, *ModelName.ToString());

	if(!FPaths::DirectoryExists(Path))
	{
		// Should never happen due to dropdown being generated from existing folder names
		UE_LOG(LogTemp, Error, TEXT("Directory does not exist: %s"), *Path);
		UGameplayStatics::GetGameInstance(WorldContextObject)->GetSubsystem<UFloraEngineLogger>()->LogMessage("Directory does not exist: " + Path);
		return nullptr;
	}

#if PLATFORM_ANDROID
	// On Android, models must be copied from the OBB to pass to the .so
	FString OutPath = FPaths::Combine(FPaths::ProjectPersistentDownloadDir(), TEXT("Models"), *ModelName.ToString());

	if (!FPaths::FileExists(OutPath))
	{
		IPlatformFile::GetPlatformPhysical().CreateDirectoryTree(*OutPath);
		IPlatformFile::GetPlatformPhysical().CopyFile(*FPaths::Combine(OutPath, TEXT("tokenizer.bin")), *FPaths::Combine(Path, TEXT("tokenizer.bin")));
		IPlatformFile::GetPlatformPhysical().CopyFile(*FPaths::Combine(OutPath, TEXT("weights.bin")), *FPaths::Combine(Path, TEXT("weights.bin")));
	}
	Path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*OutPath);
#endif

	// Set properties for the model instance
	nlm->model = _DLLInitNLM(TCHAR_TO_ANSI(*Path));

	nlm->firstLine = *FirstLine;
	nlm->secondLine = *SecondLine;
	nlm->ModelName = ModelName;

	// Load the model's input format from its json file
	nlm->inputFormat = GetModelInputFromJson(ModelName.ToString());

	UGameplayStatics::GetGameInstance(WorldContextObject)->GetSubsystem<UFloraEngineLogger>()->LogMessage("Loading model: " + ModelName.ToString());
	return nlm;
}

void UFloraEngineBPLibrary::FloraEngineDeleteNLM(UNLM *Model)
{
	if (!initialized) {
		UE_LOG(LogTemp, Error, TEXT("Flora not initialized, use the Flora Engine Init node first!"));
		return;
	}

	if (!Model || !Model->model) {
		UE_LOG(LogTemp, Warning, TEXT("Tried to delete null model!"));
	}

	_DLLDelNLM(Model->model);
	delete Model->model;
	Model->model = nullptr;
	initialized = false;
}


void UFloraEngineBPLibrary::FloraEngineInferNLM(UNLM* Model, FString Prompt)
{
	if (!initialized) {
		UE_LOG(LogTemp, Error, TEXT("Flora not initialized, use the Flora Engine Init node first!"));
		return;
	}

	_DLLInferNLM(Model->model, TCHAR_TO_ANSI(*Prompt));
}

UNLMOutput* UFloraEngineBPLibrary::FloraEngineOutputNLM(UNLM *Model)
{
	if (!initialized) {
		UE_LOG(LogTemp, Error, TEXT("Flora not initialized, use the Flora Engine Init node first!"));
		return nullptr;
	}

	if (!Model) {
		UE_LOG(LogTemp, Error, TEXT("Model is null! Try deleting it and re-making."));
		return nullptr;
	}

	if (!Model->model) {
		UE_LOG(LogTemp, Error, TEXT("Internal model is null! Try deleting it and re-making."));
		return nullptr;
	}

	UNLMOutput* output = NewObject<UNLMOutput>((UObject*)GetTransientPackage(), UNLMOutput::StaticClass());

	if (!_DLLOutputNLM) {
		UE_LOG(LogTemp, Error, TEXT("Output function is null!"));
		return nullptr;
	}

	NLMOutput* out = _DLLOutputNLM(Model->model);

	output->output = out;

	return output;
}

FString UFloraEngineBPLibrary::FloraEngineParseInput(FString prompt_input) {
	return prompt_input;
}

void UFloraEngineBPLibrary::FloraEngineParseOutput(UObject* WorldContextObject, UNLM* Model, UNLMOutput* ModelOutput, FString& output, FString& reaction, float& token_speed) {
	if (!initialized) {
		UE_LOG(LogTemp, Error, TEXT("Flora not initialized, use the Flora Engine Init node first!"));
		return;
	}
	if (!ModelOutput || !Model) {
		UE_LOG(LogTemp, Error, TEXT("Model or ModelOutput not provided."));
		return;
	}
	// Ensure null-termination of output buffer
	NLMOutput *model_out = ModelOutput->output;
	model_out->buffer[model_out->size-1] = '\0';

	// Convert to FString and split into lines
	FString output_str(UTF8_TO_TCHAR((const char*)model_out->buffer));

	TArray<FString> lines;
	output_str.ParseIntoArrayLines(lines);

	output = lines[0];
	reaction = lines[1];
	// If the reaction is guardrailed, revert to previous first line
	if (reaction.Contains(UFloraEngineSettings::GetGuardrailedReaction()))
	{
		Model->firstLine = Model->prevFirstLine;
	}
	else 
	{
		Model->secondLine = lines[0];
	}
	token_speed = model_out->token_speed;

	// Logged when parsing the output, if output is obtained but not parsed then it won't be logged
	FString LogString ="--- Result from " + Model->ModelName.ToString() + " model ---\n"+ "Output: " + output + "\nReaction: " + reaction + "\n";
	if (UFloraEngineSettings::IsTokenSpeedLogged())
	{
		LogString += "Token Speed: " + FString::SanitizeFloat(token_speed) + "\n----------------------------------------\n";
	}
	else 
	{
		LogString += "----------------------------------------\n";
	}
	UGameplayStatics::GetGameInstance(WorldContextObject)->GetSubsystem<UFloraEngineLogger>()->LogMessage(LogString);
}

TSharedPtr<FJsonObject> UFloraEngineBPLibrary::GetModelInputFromJson(FString SubFolder)
{
	// json file must have same name as subfolder
	FString Path = FPaths::Combine(*ModelRootPath, SubFolder + "/" + SubFolder + ".json");
	FString FileText;
	FFileHelper::LoadFileToString(FileText, *Path);

	// Parse JSON
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(FileText);
	TSharedPtr<FJsonObject> ResponseObj;
	FJsonSerializer::Deserialize(JsonReader, ResponseObj);

	// Get the "all_combinations"->"model_input" object and return it
	TSharedPtr<FJsonObject> ModelInput = ResponseObj->GetObjectField(TEXT("all_combinations"))->GetObjectField(TEXT("model_input"));

	return ModelInput;
}


UAsyncFloraEngineBPLibrary::UAsyncFloraEngineBPLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//UE_LOG(LogTemp, Warning, TEXT("HELLO FROM ASYNC INIT"));
}

UAsyncFloraEngineBPLibrary* UAsyncFloraEngineBPLibrary::AsyncFloraEngineBPLibrary(UObject* WorldContextObject, UNLM* model, FName ModelName, FModelInputLine InputLine1, FModelInputLine InputLine2, FModelInputLine InputLine3, FString prompt)
{
	// ModelName is not used here, but is included for the input line pin dropdowns to work correctly
	UAsyncFloraEngineBPLibrary* blueprintNode = NewObject<UAsyncFloraEngineBPLibrary>();
	blueprintNode->WorldContextObject = WorldContextObject;
	blueprintNode->Model = model;
	blueprintNode->Prompt = prompt;
	blueprintNode->Inputs = FModelInputCombination(InputLine1.LineIndex, InputLine2.LineIndex, InputLine3.LineIndex);

	// Bind to PIE end to avoid issues with async tasks trying to access deleted objects
#if WITH_EDITOR
	blueprintNode->EndPIEHandle = FEditorDelegates::EndPIE.AddLambda([blueprintNode](bool bSimulating)
		{
			blueprintNode->bExiting = true;
			FEditorDelegates::EndPIE.Remove(blueprintNode->EndPIEHandle);
		});
#endif

	return blueprintNode;
}

UAsyncFloraEngineBPLibrary* UAsyncFloraEngineBPLibrary::AsyncFloraEngineBPLibraryIndex(UObject* WorldContextObject, UNLM* model, FModelInputCombination Input, FString prompt)
{
	// ModelName is not used here, but is included for the input line pin dropdowns to work correctly
	UAsyncFloraEngineBPLibrary* blueprintNode = NewObject<UAsyncFloraEngineBPLibrary>();
	blueprintNode->WorldContextObject = WorldContextObject;
	blueprintNode->Model = model;
	blueprintNode->Prompt = prompt;
	blueprintNode->Inputs = Input;

	// Bind to PIE end to avoid issues with async tasks trying to access deleted objects
#if WITH_EDITOR
	blueprintNode->EndPIEHandle = FEditorDelegates::EndPIE.AddLambda([blueprintNode](bool bSimulating)
		{
			blueprintNode->bExiting = true;
			FEditorDelegates::EndPIE.Remove(blueprintNode->EndPIEHandle);
		});
#endif

	return blueprintNode;
}

void UAsyncFloraEngineBPLibrary::Activate()
{
	if (!initialized) {
		UE_LOG(LogTemp, Error, TEXT("Flora not initialized, use the Flora Engine Init node first!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Activate called on thread ID: %d"), FPlatformTLS::GetCurrentThreadId());

	// Run the inference on a separate thread
	Async(EAsyncExecution::Thread, [this]()
		{
			// Construct the input string based on the model's input format and the provided lines
			FString input;
			input += Model->inputFormat->GetStringField(TEXT("<BOS>"));
			input += "\n";

			FString Left, Right;
			Model->inputFormat->GetArrayField(TEXT("0"))[Inputs.FirstLinePrefixIndex]->AsString().Split(": ", &Left, &Right);
			Right.Split("<string>", &Left, &Right);
			input += Left;
			input += Model->firstLine;
			input += "\n"; 

			Model->inputFormat->GetArrayField(TEXT("1"))[Inputs.SecondLinePrefixIndex]->AsString().Split(": ", &Left, &Right);
			Right.Split("<string>", &Left, &Right);
			input += Left;
			input += Model->secondLine;
			input += "\n";

			// Third line may have USER PROMPT or not
			Model->inputFormat->GetArrayField(TEXT("2"))[Inputs.PromptPrefixIndex]->AsString().Split(": ", &Left, &Right);
			if (Right.Contains("USER PROMPT")) {
				Right.Split("USER PROMPT", &Left, &Right);
				input += *Left;
				input += Prompt;
				input += "\n";
				Model->prevFirstLine = Model->firstLine;
				Model->firstLine = Prompt;
			}else {
				input += Right;
				Model->prevFirstLine = Model->firstLine;
				Model->firstLine = Model->secondLine;
			}

			input += Model->inputFormat->GetStringField(TEXT("<RUN>"));
			
			UGameplayStatics::GetGameInstance(WorldContextObject)->GetSubsystem<UFloraEngineLogger>()->LogMessage("--- Infer called on " + Model->ModelName.ToString() + " model ---\nInput: \n" + input + "\n----------------------------------------\n");
			std::string input_std = TCHAR_TO_UTF8(*input);
			const char* to_infer = input_std.c_str();
			
			UE_LOG(LogTemp, Log, TEXT("_DLLInferNLM started on thread ID: %d"), FPlatformTLS::GetCurrentThreadId());
			_DLLInferNLM(Model->model, to_infer);
			UE_LOG(LogTemp, Log, TEXT("_DLLInferNLM completed on thread ID: %d"), FPlatformTLS::GetCurrentThreadId());

			if (!bExiting) {
				AsyncTask(ENamedThreads::GameThread, [this]()
					{
						UE_LOG(LogTemp, Log, TEXT("Post-InferNLM actions running on thread ID: %d"), FPlatformTLS::GetCurrentThreadId());
						OnCompleted.Broadcast();
						SetReadyToDestroy();
					});
			}
		});
}
