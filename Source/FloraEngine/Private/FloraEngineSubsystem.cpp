// Copyright © 2025, Databiomes Inc. All rights reserved

#include "FloraEngineSubsystem.h"
#include "FloraEngineSettings.h"
#include "FloraEngineLogger.h"
#include "FloraEngine.h"

#include "Interfaces/IPluginManager.h"
#include "Kismet/GameplayStatics.h"
#include <string>
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonValue.h"
#include "HAL/PlatformFileManager.h"


FString GetSystemErrorMessage(uint32 ErrorCode)
{
	TCHAR Buffer[256];
	FPlatformMisc::GetSystemErrorMessage(Buffer, 256, ErrorCode);
	return FString(Buffer);
}


void UFloraEngineSubsystem::Init()
{
	if (bInitialized) return;

	bInitialized = true;
	FFloraEngineModule& FloraEngineModule = FModuleManager::GetModuleChecked<FFloraEngineModule>("FloraEngine");
	void* DLLModuleFlora = FloraEngineModule.GetFloraDLLHandle();

	ModelRootPath = UFloraEngineSettings::GetModelRootPath();

#if PLATFORM_WINDOWS // may need for mac and linux
	ModelRootPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*ModelRootPath);
#endif

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

#if WITH_EDITOR
	EndPIEHandle = FEditorDelegates::EndPIE.AddLambda([this](bool bSimulating)
		{
			for (FModelAsyncPrompt* Prompt : ActivePrompts)
			{
				Prompt->bExiting = true;
			}
		});
#endif
}

void UFloraEngineSubsystem::Deinitialize()
{
	Super::Deinitialize();
#if WITH_EDITOR
	FEditorDelegates::EndPIE.Remove(EndPIEHandle);
#endif
}

UNLM* UFloraEngineSubsystem::InitNLM(UObject* WorldContextObject, FName ModelName, FString FirstLine, FString SecondLine)
{
	if (!bInitialized) {
		UE_LOG(LogTemp, Error, TEXT("Flora not initialized, use the Flora Engine Init node first!"));
		return nullptr;
	}

	UNLM* nlm = NewObject<UNLM>((UObject*)GetTransientPackage(), UNLM::StaticClass());

	// Set path using the model name
	FString Path = FPaths::Combine(*ModelRootPath, *ModelName.ToString());

	if (!FPaths::DirectoryExists(Path))
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
	nlm->Model = _DLLInitNLM(TCHAR_TO_ANSI(*Path));
	nlm->FirstLine = *FirstLine;
	nlm->SecondLine = *SecondLine;
	nlm->ModelName = ModelName;

	// Setup model from json
	SetupModelFromJson(WorldContextObject, nlm);

	UGameplayStatics::GetGameInstance(WorldContextObject)->GetSubsystem<UFloraEngineLogger>()->LogMessage("Loading model: " + ModelName.ToString());
	return nlm;
}

void UFloraEngineSubsystem::AsyncInfer(FModelAsyncPrompt* ModelPrompt, const TFunction<void()>& Fn)
{
	if (!bInitialized) {
		UE_LOG(LogTemp, Error, TEXT("Flora not initialized, use the Flora Engine Init node first!"));
		return;
	}

	if (!ModelPrompt->Model || !ModelPrompt->Model->Model) {
		UE_LOG(LogTemp, Error, TEXT("Model is null on infer call."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Activate called on thread ID: %d"), FPlatformTLS::GetCurrentThreadId());
	ActivePrompts.Add(ModelPrompt);
	// Run the inference on a separate thread
	Async(EAsyncExecution::Thread, [=, this]()
		{
			FString Input;
			TArray<TSharedPtr<FJsonValue>> ModelInputs = ModelPrompt->Model->InputFormat->GetObjectField(ModelPrompt->InstructionLine.ToString())->GetArrayField(TEXT("input"));
			ModelPrompt->Model->LastInstructionLine = ModelPrompt->InstructionLine.ToString();

			// Construct the input string based on the model's input format and the provided lines
			Input += ModelInputs[0]->AsString(); // <BOS>
			TSharedPtr<FJsonObject> InputLine;

			FString Left, Right;
			ModelInputs[1]->AsString().Split("<string>", &Left, &Right);
			Input += Left;
			Input += ModelPrompt->Model->FirstLine;
			Input += Right;

			ModelInputs[2]->AsString().Split("<string>", &Left, &Right);
			Right.Split("<string>", &Left, &Right);
			Input += Left;
			Input += ModelPrompt->Prompt;
			Input += Right;

			// Currently only basic type exists
			if (ModelPrompt->Model->InputFormat->GetObjectField(ModelPrompt->InstructionLine.ToString())->GetStringField(TEXT("type")) == "basic") {
				Input += ModelInputs[3]->AsString();
				ModelPrompt->Model->PrevFirstLine = ModelPrompt->Model->FirstLine;
				ModelPrompt->Model->FirstLine = ModelPrompt->Model->SecondLine;
			}

			Input += ModelInputs[4]->AsString(); // <RUN>

			UGameplayStatics::GetGameInstance(ModelPrompt->WorldContextObject)->GetSubsystem<UFloraEngineLogger>()->LogMessage("--- Infer called on " + ModelPrompt->Model->ModelName.ToString() + " model ---\nInput: \n" + Input + "\n----------------------------------------\n");
			std::string InputStd = TCHAR_TO_UTF8(*Input);
			const char* ToInfer = InputStd.c_str();

			UE_LOG(LogTemp, Log, TEXT("_DLLInferNLM started on thread ID: %d"), FPlatformTLS::GetCurrentThreadId());
			_DLLInferNLM(ModelPrompt->Model->Model, ToInfer);
			UE_LOG(LogTemp, Log, TEXT("_DLLInferNLM completed on thread ID: %d"), FPlatformTLS::GetCurrentThreadId());

			if (!ModelPrompt->bExiting) {
				AsyncTask(ENamedThreads::GameThread, Fn);
			}
		});
}

void UFloraEngineSubsystem::GetOutput(UObject* WorldContextObject, UNLM* Model, FString& Output, uint8& InstructionIndex, uint8& ReactionIndex, FString& Reaction, float& TokenSpeed)
{
	if (!bInitialized) {
		UE_LOG(LogTemp, Error, TEXT("Flora not initialized, use the Flora Engine Init node first!"));
		return;
	}
	if (!Model) {
		UE_LOG(LogTemp, Error, TEXT("Model is null! Try deleting it and re-making."));
		return;
	}
	UNLMOutput* ModelOutput = FloraEngineOutputNLM(Model);

	// Ensure null-termination of output buffer
	NLMOutput* ModelOut = ModelOutput->Output;
	ModelOut->Buffer[ModelOut->Size - 1] = '\0';

	// Convert to FString and split into lines
	FString OutputStr(UTF8_TO_TCHAR((const char*)ModelOut->Buffer));

	TArray<FString> Lines;
	OutputStr.ParseIntoArrayLines(Lines);
	Output = Lines[0];
	Reaction = Lines[1];

	// If the reaction is guardrailed, revert to previous first line
	if (Reaction.Contains("<UNK>"))
	{
		Model->FirstLine = Model->PrevFirstLine;
	}
	else
	{
		Model->SecondLine = Lines[0];
	}
	TokenSpeed = ModelOut->TokenSpeed;

	// Logged when parsing the output, if output is obtained but not parsed then it won't be logged
	FString LogString = "--- Result from " + Model->ModelName.ToString() + " model ---\n" + "Output: " + Output + "\nReaction: " + Reaction + "\n";
	if (UFloraEngineSettings::IsTokenSpeedLogged())
	{
		LogString += "Token Speed: " + FString::SanitizeFloat(TokenSpeed) + "\n----------------------------------------\n";
	}
	else
	{
		LogString += "----------------------------------------\n";
	}
	UGameplayStatics::GetGameInstance(WorldContextObject)->GetSubsystem<UFloraEngineLogger>()->LogMessage(LogString);

	// Decrypt reaction
	if (Model->OutputDecryptMap.Contains(Reaction))
		Reaction = Model->OutputDecryptMap.FindRef(Reaction);

	ReactionIndex = Model->InstructionReactionMap.Find(Model->LastInstructionLine)->Find(Reaction);
	InstructionIndex = Model->InstructionIndexMap.FindRef(Model->LastInstructionLine);
}

void UFloraEngineSubsystem::SetupModelFromJson(UObject* WorldContextObject, UNLM* Model)
{
	// json file must have same name as subfolder
	FString Path = UFloraEngineSettings::GetTemplatePath(Model->ModelName);
	FString FileText;
	FFileHelper::LoadFileToString(FileText, *Path);

	// Parse JSON
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(FileText);
	TSharedPtr<FJsonObject> ResponseObj;
	FJsonSerializer::Deserialize(JsonReader, ResponseObj);

	const TSharedPtr<FJsonObject>* ModelInput = nullptr;
	// Get the "instructions" object
	if (ResponseObj->TryGetObjectField(TEXT("instructions"), ModelInput)) {
		Model->InputFormat = ModelInput->ToSharedRef();
		TSharedPtr<FJsonObject> AllResults = ResponseObj->GetObjectField(TEXT("tokens"))->GetObjectField(TEXT("output"));
		for (auto CurrJsonValue = AllResults->Values.CreateConstIterator(); CurrJsonValue; ++CurrJsonValue)
		{
			Model->AllReactions.Add(CurrJsonValue->Value->AsString(), Model->AllReactions.Num()); // Map reaction to its index
			Model->OutputDecryptMap.Add(CurrJsonValue->Value->AsString(), CurrJsonValue->Key);
			if (UFloraEngineSettings::IsDecryptLog()) {
				UGameplayStatics::GetGameInstance(WorldContextObject)->GetSubsystem<UFloraEngineLogger>()->AddDecryptToken(CurrJsonValue->Value->AsString(), CurrJsonValue->Key);
			}
		}
		if (UFloraEngineSettings::IsDecryptLog()) {
			AllResults = ResponseObj->GetObjectField(TEXT("tokens"))->GetObjectField(TEXT("input"));
			for (auto CurrJsonValue = AllResults->Values.CreateConstIterator(); CurrJsonValue; ++CurrJsonValue) {
				UGameplayStatics::GetGameInstance(WorldContextObject)->GetSubsystem<UFloraEngineLogger>()->AddDecryptToken(CurrJsonValue->Value->AsString(), CurrJsonValue->Key);
			}
		}

		for (auto CurrInstruction = ModelInput->Get()->Values.CreateConstIterator(); CurrInstruction; ++CurrInstruction)
		{
			TArray<FString> InstructionResults;
			TArray<TSharedPtr<FJsonValue>> ResultsArray = CurrInstruction->Value->AsObject()->GetArrayField(TEXT("output"));
			for (const TSharedPtr<FJsonValue>& Value : ResultsArray) {
				TArray<FString> Lines;
				Value->AsString().ParseIntoArrayLines(Lines);
				InstructionResults.Add(Lines[1]);
			}
			Model->InstructionIndexMap.Add(CurrInstruction->Key, Model->InstructionIndexMap.Num()); // Map instruction name to its index
			Model->InstructionReactionMap.Add(CurrInstruction->Key, InstructionResults); // Map instruction name to its possible reactions
		}

		return;
	}

	UE_LOG(LogTemp, Error, TEXT("Could not find model input format in json file: %s"), *Path);
	return;
}

UNLMOutput* UFloraEngineSubsystem::FloraEngineOutputNLM(UNLM* Model)
{
	if (!bInitialized) {
		UE_LOG(LogTemp, Error, TEXT("Flora not initialized, use the Flora Engine Init node first!"));
		return nullptr;
	}

	if (!Model) {
		UE_LOG(LogTemp, Error, TEXT("Model is null! Try deleting it and re-making."));
		return nullptr;
	}

	if (!Model->Model) {
		UE_LOG(LogTemp, Error, TEXT("Internal model is null! Try deleting it and re-making."));
		return nullptr;
	}

	UNLMOutput* Output = NewObject<UNLMOutput>((UObject*)GetTransientPackage(), UNLMOutput::StaticClass());

	if (!_DLLOutputNLM) {
		UE_LOG(LogTemp, Error, TEXT("Output function is null!"));
		return nullptr;
	}

	NLMOutput* Out = _DLLOutputNLM(Model->Model);

	Output->Output = Out;

	return Output;
}

UAsyncFloraEngineBPLibrary::UAsyncFloraEngineBPLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAsyncFloraEngineBPLibrary* UAsyncFloraEngineBPLibrary::AsyncFloraEngineBPLibraryInstructionLine(UObject* WorldContextObject, UNLM* Model, FName ModelName, FName InstructionLine, FString Prompt)
{
	// ModelName is not used here, but is included for the input line pin dropdowns to work correctly
	UAsyncFloraEngineBPLibrary* BlueprintNode = NewObject<UAsyncFloraEngineBPLibrary>();
	BlueprintNode->ModelPrompt.WorldContextObject = WorldContextObject;
	BlueprintNode->ModelPrompt.Model = Model;
	BlueprintNode->ModelPrompt.Prompt = Prompt;
	BlueprintNode->ModelPrompt.InstructionLine = InstructionLine;

	// Bind to PIE end to avoid issues with async tasks trying to access deleted objects
#if WITH_EDITOR
	BlueprintNode->ModelPrompt.EndPIEHandle = FEditorDelegates::EndPIE.AddLambda([BlueprintNode](bool bSimulating)
		{
			BlueprintNode->ModelPrompt.bExiting = true;
			FEditorDelegates::EndPIE.Remove(BlueprintNode->ModelPrompt.EndPIEHandle);
		});
#endif

	return BlueprintNode;
}

void UAsyncFloraEngineBPLibrary::Activate()
{
	UFloraEngineSubsystem* FloraEngineSubsystem = UGameplayStatics::GetGameInstance(ModelPrompt.WorldContextObject)->GetSubsystem<UFloraEngineSubsystem>();
	FloraEngineSubsystem->AsyncInfer(&ModelPrompt, [this]()
		{
			if (!ModelPrompt.bExiting) {
				UE_LOG(LogTemp, Log, TEXT("Post-InferNLM actions running on thread ID: %d"), FPlatformTLS::GetCurrentThreadId());
				OnCompleted.Broadcast();
				SetReadyToDestroy();
			}
		});
}