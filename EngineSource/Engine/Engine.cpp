#include "Engine.h"
#include "Subsystem/VideoSubsystem.h"
#include "Subsystem/EditorSubsystem.h"
#include "Subsystem/InputSubsystem.h"
#include "Subsystem/SceneSubsystem.h"
#include "Engine/Internal/WorkingDirectory.h"
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

	Engine* New = new Engine();
	Instance = New;

	New->LoadSubsystem(new VideoSubsystem());
	New->LoadSubsystem(new InputSubsystem());
	New->LoadSubsystem(new SceneSubsystem());

#ifdef EDITOR
	New->LoadSubsystem(new EditorSubsystem());
#endif

	return New;
}

void Engine::Run()
{
	while (!ShouldQuit)
	{
		for (ISubsystem* System : LoadedSystems)
		{
			System->Update();
		}
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
