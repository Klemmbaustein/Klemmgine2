# Klemmgine 2

> [!WARNING]
> The engine is still in development. It isn't stable yet.

A full rewrite of my game "Klemmgine" game engine.
A very lightweight (the editor executable is 9MB, a non editor is 3.5MB) 3D game engine
written in C++ using OpenGL for rendering. It currently runs on Windows
and Linux.

## Features:

### High level

- Pretty fast rendering (can easily maintain 1000+ frames per second on a 3060 TI)
  and editor (Starting the editor and opening a project usually takes around 100ms)
- A customizable graphical editor to edit 3d scenes, assets and manage and create projects.
- A custom scripting language inspired mostly by C#, integrated with the UI definition
  language [from my UI library](https://github.com/Klemmbaustein/KlemmUI)
  and a script editor for this built into the editor.
- Custom shader system and a material system that control uniforms for these shaders.
- Built in graphical effects like real time shadows, bloom,
  ambient occlusion, anti aliasing and a robust post processing system that
  can easily support more post process effects.
- A physics/collision system powered by Jolt Physics.

#### (WIP)

- (Very WIP) Plugin support.
- A built in command line to interact with the engine directly. (Currently limited, not many commands)
- (Soon, for now not included by default in builds) A "Visual Studio live share" like feature
  for pair coding and live collaboration when editing a project.

### Low level

- A virtual filesystem allowing for asset compression and loading assets from various sources
  (Even from a remote server).
- A custom serialization format that has a text (for source control and manual editing)
  and binary representation, and can also be partially serialized to JSON
  (see EngineSource/Core/File/JsonSerializer.h)

### TODO/Missing features:
- No audio yet!!!
- Missing some graphical effects from Klemmgine 1,
  such as baked lighting and more types of light sources.
- Fix "Editor Server" live collaboration feature.
- No multi player features yet, unlike Klemmgine 1.
- Better general and scripting documentation.

## Building

The project is built using CMake.

### Windows
- The Visual Studio "Desktop development with C++" workload is enough to compile the engine.
- Configure the project with CMake (example: `cmake -S . -B out/build/editor/ -G Ninja -DOPTION=ON`)

  | Name                  | Default | Description |
  |-----------------------|---------|-------------|
  | USE_WINDOWS_SUBSYSTEM | ON | When OFF, the engine compiles with the CONSOLE subsystem, showing a console window for logging. |
  | ENGINE_EDITOR         | OFF | Set to ON to build the editor. |
  | ENGINE_RUN_MSVC_CODE_ANALYSIS | OFF | Set to ON to run a MSVC code analysis on the project. |
  | ENGINE_CSHARP         | OFF | Include experimental C# scripting plugin (currently very unfinished) |

### Linux
- All requirements for SDL3 and GLEW should be installed.
- Configure the project with CMake (example: `cmake -S . -B out/build/editor/ -G Ninja -DOPTION=ON`)

  | Name                  | Default | Description |
  |-----------------------|---------|-------------|
  | ENGINE_DYNAMIC_SYMBOLS | OFF | Compiles with dynamic symbols (-rdynamic), increasing executable size but showing function names in stack traces.
  | ENGINE_EDITOR         | OFF | Set to ON to build the editor. |
  | ENGINE_CSHARP         | OFF | Include experimental C# scripting plugin (currently very unfinished) |
