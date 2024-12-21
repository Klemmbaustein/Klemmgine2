#include "Engine.h"
#include "Subsystem/VideoSubsystem.h"
#include "Subsystem/EditorSubsystem.h"
#include "Subsystem/InputSubsystem.h"
#include "Subsystem/ConsoleSubsystem.h"
#include "Subsystem/SceneSubsystem.h"
#include "Internal/WorkingDirectory.h"
#include "MainThread.h"
#include "File/Resource.h"
#include "Error/EngineError.h"
using namespace engine;
using namespace engine::subsystem;

Engine* Engine::Instance = nullptr;

Engine::Engine()
{
}

Engine* Engine::Init()
{
	if (Instance)
	{
		return Instance;
	}

	internal::AdjustWorkingDirectory();

	Instance = new Engine();
	
	error::InitForThread("Main");
	thread::IsMainThread = true;
	Reflection::Init();
	resource::ScanForAssets();

	Instance->LoadSubsystem(new ConsoleSubsystem());
	Instance->LoadSubsystem(new VideoSubsystem());
	Instance->LoadSubsystem(new InputSubsystem());
	Instance->LoadSubsystem(new SceneSubsystem());

#ifdef EDITOR
	Instance->LoadSubsystem(new EditorSubsystem());
#endif

	return Instance;
}

void Engine::Run()
{
	while (!ShouldQuit)
	{
		for (ISubsystem* System : LoadedSystems)
		{
			System->Update();
		}
		thread::MainThreadUpdate();
		for (ISubsystem* System : LoadedSystems)
		{
			System->RenderUpdate();
		}
	}

	for (int64 i = LoadedSystems.size() - 1; i >= 0; i--)
	{
		delete LoadedSystems[i];
	}
	LoadedSystems.clear();

	Instance = nullptr;

	delete this;
}

void Engine::LoadSubsystem(ISubsystem* NewSubsystem)
{
}
