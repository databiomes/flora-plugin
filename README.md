# Flora Engine

## Overview
An open plugin for running Databiomes Language Models in Unreal Engine

## Installation
For blueprint only projects, "FloraEngine.zip" can be downloaded from the latest release and extracted to the marketplace plugins folder ("/UE_5.6/Engine/Plugins/Marketplace/") for the project to package properly.</br>  

Alternatively, the source code can be packaged from a C++ project first and then added to the marketplace plugins folder.</br>

<img width="877" height="217" alt="image" src="https://github.com/user-attachments/assets/2d6e1c9f-52b2-4da4-8206-e5a5c814d024" /></br>

Adding plugin to marketplace plugins is recommended for both types of projects.</br>
For C++ Unreal Engine projects plugin CAN be included in the project directory under Plugins instead.

Files for the models (tokenizer.bin and weights.bin) must be located in a folder within the plugin (default: Content/Models/, can be adjusted in the settings) with each model contained in a subfolder along with a template .json file. The .json file must have the same name as the model name used for the folder.</br>
The library file (flora.dll for Windows or libflora.so for Android) must also be located in the Content folder.

Example file structure:
```
📦 FloraEngine
└─ 📁Content
   ├─ 📄flora.dll
   ├─ 📄flora.so
   └─ 📁Models
      ├─ 📁CheshireCat
      │  ├─ 📄CheshireCat.json
      │  ├─ 📄tokenizer.bin
      │  └─ 📄weights.bin
      ├─ 📁MadHatter
      │  ├─ 📄MadHatter.json
      │  ├─ 📄tokenizer.bin
      │  └─ 📄weights.bin
      └─ 📁MarchHare
         ├─ 📄MarchHare.json
         ├─ 📄tokenizer.bin
         └─ 📄weights.bin
```
