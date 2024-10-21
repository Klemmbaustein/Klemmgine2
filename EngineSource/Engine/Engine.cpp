#include "Engine.h"
#include "Subsystem/VideoSubsystem.h"
#include "Subsystem/EditorSubsystem.h"
#include "Subsystem/InputSubsystem.h"
#include "Subsystem/SceneSubsystem.h"
#include <kui/Window.h>

using namespace engine;

Engine* Engine::Instance = nullptr;

Engine::Engine()
{
	using namespace kui;
}

Engine* Engine::Init()
{
	using namespace subsystem;

	if (Instance)
	{
		return Instance;
	}

	Engine* New = new Engine();
	Instance = New;

	New->LoadSubsystem(new VideoSubsystem());
	New->LoadSubsystem(new SceneSubsystem());
	New->LoadSubsystem(new InputSubsystem());

#ifdef EDITOR
//	New->LoadSubsystem(new EditorSubsystem());
#endif

	return New;
}

void Engine::Run()
{
	while (!ShouldQuit)
	{
		for (subsystem::Subsystem* System : LoadedSystems)
		{
			System->Update();
		}
		for (subsystem::Subsystem* System : LoadedSystems)
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

void engine::Engine::LoadSubsystem(subsystem::Subsystem* NewSubsystem)
{
}
