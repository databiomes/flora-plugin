# Flora Engine

## Overview
An open plugin for running Databiomes Language Models in Unreal Engine.</br>
An addon plugin is in the works and will be available to provide Automatic Speech Regognition (ASR) using Whisper and Text to Speech (TTS) using Kokoro in the near future.

## Installation
For blueprint only projects, "FloraEngine.zip" can be downloaded from the latest release and extracted to the marketplace plugins folder ("UE_5.6/Engine/Plugins/Marketplace/"). </br>  

Alternatively, the source code can be packaged from a C++ project first and then added to the marketplace plugins folder.</br>

<picture> <img width="877" height="217" alt="image" src="https://github.com/user-attachments/assets/2d6e1c9f-52b2-4da4-8206-e5a5c814d024"/></picture></br>

Adding plugin to marketplace plugins is recommended for both types of projects.</br>
For C++ Unreal Engine projects plugin CAN be included in the project directory under Plugins instead.

Files for the models (tokenizer.bin and weights.bin) must be located in a folder within the plugin (default: Content/Models/, can be adjusted in the settings) with each model contained in a subfolder along with a template .json file. The .json file must have the same name as the model name used for the folder with "_template" appended to the end of the name.</br>
The library files (flora.dll for Windows or libflora.so for Android) are not included an must be located in the Source/ThirdParty/bin/Win64 or Source/ThirdParty/bin/Android respectively.

Example file structure:
```
📦 FloraEngine
├─ 📁Source
│  └─ 📁ThirdParty
│     └─ 📁bin
│        ├─ 📁Android
|        |  └─ 📄libflora.so
│        └─ 📁Win64
|           └─ 📄flora.dll
└─ 📁Content
   └─ 📁Models
      ├─ 📁CheshireCat
      │  ├─ 📄CheshireCat_template.json
      │  ├─ 📄tokenizer.bin
      │  └─ 📄weights.bin
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
After the plugin is installed with the model files in the correct locations, Flora Engine blueprint nodes will be available for use and plugin settings will appear in the project settings. </br>
First, navigate to the Flora Engine settings in Edit > Project Settings > Plugins > Flora Engine. From here you will be able to customize features of the logger as well as Flora Engine itself. The "Model Root Path" must be set to the root folder where the Flora models can be found. Once set, the "Generate Reactions" button can be used to create enum assets based on the possible outputs of each model as well as the instructions they are mapped to. </br></br>
<picture> <img width="660" height="507" alt="image" src="https://github.com/user-attachments/assets/1151be9e-b672-43b2-bb5f-9c12cf7227be" /> </picture></br>

The node "Initialize Flora Lib" must be called before using the other nodes of the plugin. Create a new actor or navigate to the level blueprint and connect the node to begin play. Next, each instance of models to be used must be initialized using the "Initialize NLM Using Model" node. With this node, there will be a dropdown called "Model Name" that, if model files and settings are properly set up, will show a list of your available models. If the models are not appearing, try compiling the blueprint again to update the node (additionally if there are any changes to the models folder, the blueprint will need to be recompiled). The model can be provided with a "First Line" and "Second Line" prompt for its initial inference. </br></br>
<picture> <img width="778" height="254" alt="image" src="https://github.com/user-attachments/assets/e65fcd6d-4109-4560-8706-d7531bd2a2a1" /> </picture></br>

In order to infer the model you can use the node "AsyncInfer Constructor". This node takes in the model as a parameter along with the name of the model given when the NLM was initialized. Setting the "Model Name" and compiling will update the "Instruction Line" dropdown and populate it with the available instruction prompts according to the template file. There is also a "Prompt" field for extended instructions. </br></br>
<picture> <img width="435" height="292" alt="image" src="https://github.com/user-attachments/assets/1d90a873-9655-4143-a5cd-768ff74ca2c6" /> </picture></br>

The output must then be retrieved from the NLM using the "Get NLM Output" node (note: this node must be used on the model after the AsyncInfer node has completed). This node will output the generated text as "Output", the "Reaction Index" which can be used along side the generated reaction enums in the settings to create a state machine, the "Reaction" value itself and the "Token Speed". </br></br>
<picture> <img width="842" height="249" alt="image" src="https://github.com/user-attachments/assets/7f8c9686-e520-428d-bf2d-94106bd0d2e0" /> </picture></br>
