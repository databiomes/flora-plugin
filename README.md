# Flora Engine

## Table of Contents
- [Overview](#overview)
- [Installation](#installation)
- [Getting Started](#getting-started)
	- [Blueprints](#blueprints)
	- [State Machines](#state-machines)
 	- [C++](#c)
- [Supported Devices](#supported-devices)

## Overview
An open plugin for running Databiomes Language Models in Unreal Engine.</br>
An addon plugin is available to provide Automatic Speech Recognition (ASR) using Whisper and Text to Speech (TTS) using Kokoro.

## Installation
For blueprint only projects, `FloraEngine.zip` can be downloaded from the [latest release](https://github.com/databiomes/flora-plugin/releases/latest) and extracted to the marketplace plugins folder `"UE_<VERSION NUMBER>/Engine/Plugins/Marketplace/"`. </br>  
For C++ Unreal Engine projects plugin can be included in the project directory under Plugins instead if desired.

Files for the models (`tokenizer.bin` and `weights.bin`) must be located in a folder within the plugin (default: `Content/Models/`, can be adjusted in the settings) with each model contained in a subfolder along with a template .json file. The .json file must have the same name as the model name used for the folder with "_template" appended to the end of the name.</br>
The library files (`flora.dll` for Windows or `libflora.so` for Android) are not included and must be placed in the `Source/ThirdParty/bin/Win64` or `Source/ThirdParty/bin/Android` respectively and the dll must also be placed in `Binaries/Win64`.</br>
Each version of the `flora.dll` must be placed in the respective folder for the device it is for.</br>

Library files can be downloaded from https://app.databiomes.com.

Note: `weights.xml` must also be included with `weights.bin` for Intel NPU models. Currently Flora is unsupported for AMD NPU and will run on the CPU if selected (Windows CPU dll must be placed in the amd_npu folder). 

Example file structure:
```
📦 FloraEngine
├─ 📁Binaries
|  └─ 📁Win64
│     ├─ 📁amd_npu
|     |  └─ 📄flora.dll
│     ├─ 📁cpu
|     |  └─ 📄flora.dll
│     └─ 📁intel_npu
|        ├─ 📄flora.dll
|        └─ 📄openvino.dll
├─ 📁Source
│  └─ 📁ThirdParty
│     └─ 📁bin
│        ├─ 📁Android
|        |  └─ 📄libflora.so
│        └─ 📁Win64
│           ├─ 📁amd_npu
|           |  └─ 📄flora.dll
│           ├─ 📁cpu
|           |  └─ 📄flora.dll
│           └─ 📁intel_npu
|               ├─ 📄flora.dll
|               └─ 📄openvino.dll
└─ 📁Content
   └─ 📁Models
      ├─ 📁CheshireCat
      │  ├─ 📄CheshireCat_template.json
      │  ├─ 📄tokenizer.bin
      │  ├─ 📄weights.bin
      │  └─ 📄weights.xml (Only for Intel NPU builds)
      ├─ 📁MadHatter
      │  ├─ 📄MadHatter_template.json
      │  ├─ 📄tokenizer.bin
      │  └─ 📄weights.bin
      └─ 📁MarchHare
         ├─ 📄MarchHare_template.json
         ├─ 📄tokenizer.bin
         └─ 📄weights.bin
```
## Getting Started
After the plugin is installed and enabled with the model files in the correct locations, `Flora Engine` blueprint nodes will be available for use and plugin settings will appear in the project settings. </br>
First, navigate to the Flora Engine settings in `Edit > Project Settings > Plugins > Flora Engine`. From here you will be able to customize features of the logger as well as `Flora Engine`itself. The `Model Root Path` must be set to the root folder where the Flora models can be found. 
Once set, the `Generate Reactions` and `Generate States` buttons can be used to create assets based on the possible outputs of each model. For a more detailed explanation, click [here](#state-machines)</br>
The settings also allow you to select the device you are targeting. Changing devices will prompt an editor restart.</br></br>
<picture> <img alt="image" src="https://github.com/user-attachments/assets/0c6b43b3-1bcc-4171-a3b3-20a3c8d7b908" /> </picture></br>

### Blueprints
The node `Initialize Flora Lib` must be called before using the other nodes of the plugin. Create a new actor or navigate to the level blueprint, get the Flora Engine Subsystem and connect the initialize node to `BeginPlay`. Next, each instance of models to be used must be initialized using the `Initialize NLM Using Model` node. With this node, there will be a dropdown called `Model Name` that, if model files are properly located, will show a list of your available models. If the models are not appearing, try compiling the blueprint again to update the node (additionally if there are any changes to the models folder, the blueprint will need to be recompiled). The model can be provided with a list of input lines for inference. </br></br>
<picture> <img alt="image" src="https://github.com/user-attachments/assets/e7b0ce88-8384-4779-9006-7c19984a33fc" /> </picture></br>

In order to infer the model you can use the node `AsyncInfer Constructor`. This node takes in the model as a parameter along with the name of the model given when the NLM was initialized. Setting the `Model Name` and compiling will update the `Instruction Line` dropdown and populate it with the available instruction prompts according to the template file. There is also a `Prompt` field for extended instructions. </br></br>
<picture> <img alt="image" src="https://github.com/user-attachments/assets/1d90a873-9655-4143-a5cd-768ff74ca2c6" /> </picture></br>

The output must then be retrieved from the NLM using the `Get NLM Output` node (note: this node must be used on the model after the `AsyncInfer Constructor` node has completed). This node will output the generated text as `Output`, the "Reaction Index" which can be used along side the generated reaction enums in the settings to create a state machine, the `Reaction` value itself and the `Token Speed`. </br></br>
<picture> <img alt="image" src="https://github.com/user-attachments/assets/1f9e1716-15ef-4ba8-926f-f8549f0a4619" /> </picture></br>


### State Machines
The `Generate Reactions` button in the settings can be used to build a state machine with `Flora` models created using `MTP`. The button will open an interface to select where the generated files will be placed. `Generate instruction specific reactions` will create a separate ENUM for the outputs of each instruction. These ENUMs can then be used similarly to the example above. </br></br>
<picture> <img alt="image" src="https://github.com/user-attachments/assets/b310eb9c-b71a-4bbf-9563-d89f0fdff454" /> </picture></br>

For `Flora` models created with the `CSV` interface, a dedicated state machine can be built. The `Generate States` button will open an interface to select the destination for the generated assets. If desired, the user may specify which models to generate state structures and datatables for. </br></br>
<picture> <img alt="image" src="https://github.com/user-attachments/assets/88dc237b-d94f-443b-86bf-a453227a8d29" /> </picture></br>

Once generated the structure will by default have a string variable and the datatable storing the structure will have a row for each state possible with the selected model. The structure can be modified to include any assets that may be required for each state of the model, for example, a `Sound Wave Asset`. </br>
Then the datatable can be populated with the desired values for each state. </br></br>
<picture> <img alt="image" src="https://github.com/user-attachments/assets/9ae53b9a-3958-4223-abd4-ac5a3c85c7d5" /> </picture></br></br>
<picture> <img alt="image" src="https://github.com/user-attachments/assets/c3adedf8-0a33-41f8-8d45-9b33386b8707" /> </picture></br>

The datatable rows can then be accessed in blueprints with the output of the Flora model to access the corresponding data for each state. </br></br>
<picture> <img alt="image" src="https://github.com/user-attachments/assets/0b3cf62a-31a8-45d1-a46c-4c0303461c5a" /> </picture></br>


### C++
For a minimal use case example of `Flora Engine` in C++ the following steps can be used:
1. Create a C++ Actor from the content browser
2. In the `.Build.cs` of the C++ project add `"FloraEngine"` to `PublicDependencyModuleNames`
3. Add `#include "FloraEngineSubsystem"` in your Actor's cpp or header file
4. In `BeginPlay()` get a reference to the `FloraEngineSubsystem`, initialize it and initialize a model. An array of initial input lines may be included if the model requires it (models generated from a .csv do not require these inputs)
5. Create a `FModelAsyncPrompt` to use with the model. Prompt is the desired input to the model. The instruction line must be chosen from the available instructions for the model found in the .json template file.
6. Call `AsyncInfer` on the subsystem using the `FModelAsyncPrompt`. A function delegate must be provided that will run on the game thread after inference.
7. After inference is complete, call `GetOutput` with the created model to retrieve the output and reaction.

Example: </br>
```
UFloraEngineSubsystem* FloraSubsystem = GetGameInstance()->GetSubsystem<UFloraEngineSubsystem>();
FloraSubsystem->Init();
UNLM* Model = FloraSubsystem->InitNLM(this, "ModelName", TArray<FString>{});

FModelAsyncPrompt* Prompt = new FModelAsyncPrompt{
				.WorldContextObject = this,
				.Model = Model,
				.Prompt = "Input prompt",
				.InstructionLine = "StateMachineInstruction"
};

FloraSubsystem->AsyncInfer(Prompt, [this, Prompt]() {
    uint8 InstructionIndex, ReactionIndex;
    FString Output, Reaction;
    float TokenSpeed;
    FloraSubsystem->GetOutput(this, Prompt->Model, Output, InstructionIndex, ReactionIndex, Reaction, TokenSpeed);

    UE_LOG(LogTemp, Warning, TEXT("Output: %s"), Output);
    UE_LOG(LogTemp, Warning, TEXT("Reaction: %s"), Reaction);
	});
```
## Supported Devices
Currently the Flora Engine plugin is able to run inference on the CPU for all Windows devices. </br>
Flora is also available to run on the NPU for OpenVino supported devices.</br>
NPU drivers must be installed prior to use.</br>
