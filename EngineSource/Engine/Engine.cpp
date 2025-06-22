#include "Engine.h"
#include "Editor/Editor.h"
#include "Subsystem/VideoSubsystem.h"
#include "Subsystem/EditorSubsystem.h"
#include "Subsystem/InputSubsystem.h"
#include "Subsystem/ConsoleSubsystem.h"
#include "Subsystem/SceneSubsystem.h"
#include "Subsystem/PluginSubsystem.h"
#include "Internal/WorkingDirectory.h"
#include "MainThread.h"
#include "Core/ThreadPool.h"
#include "File/Resource.h"
#include "Core/Error/EngineError.h"
#include <kui/App.h>
#include <Engine/Debug/TimeLogger.h>
#include <Core/LaunchArgs.h>
using namespace engine;
using namespace engine::subsystem;

Engine* Engine::Instance = nullptr;
bool Engine::IsPlaying = false;
bool Engine::GameHasFocus = false;


Engine::Engine()
{
}

Engine* Engine::Init()
{
	if (Instance)
	{
		return Instance;
	}

	debug::TimeLogger StartupTime{ "Engine started" };

	internal::AdjustWorkingDirectory();

	Instance = new Engine();

	Instance->InitSystems();

	Instance->LoadSubsystem(new ConsoleSubsystem());
	Instance->LoadSubsystem(new PluginSubsystem());
	Instance->LoadSubsystem(new VideoSubsystem());
	Instance->LoadSubsystem(new InputSubsystem());
	Instance->LoadSubsystem(new SceneSubsystem());

#ifdef EDITOR
	Instance->LoadSubsystem(new EditorSubsystem());
#else
	IsPlaying = true;
	GameHasFocus = true;
#endif


	return Instance;
}

void Engine::Run()
{
	while (!ShouldQuit)
	{
#ifdef EDITOR
		if (input::IsRMBClicked && !editor::IsActive())
		{
			LoadSubsystem(new EditorSubsystem());
		}
#endif

		for (Subsystem* System : LoadedSystems)
		{
			System->Update();
		}
		Subsystem::UpdateUnloading();
		thread::MainThreadUpdate();
		for (Subsystem* System : LoadedSystems)
		{
			System->RenderUpdate();
		}
	}

	for (int64 i = LoadedSystems.size() - 1; i >= 0; i--)
	{
		delete LoadedSystems[i];
	}
	LoadedSystems.clear();

	ThreadPool::FreeDefaultThreadPool();
	Instance = nullptr;
	IsPlaying = false;
	delete this;
}

void Engine::LoadSubsystem(Subsystem* NewSubsystem)
{
	if (typeid(*NewSubsystem) == typeid(ConsoleSubsystem))
	{
		for (auto& i : LoadedSystems)
		{
			i->RegisterCommands(static_cast<ConsoleSubsystem*>(NewSubsystem));
		}
		return;
	}

	ConsoleSubsystem* ConsoleSys = GetSubsystem<ConsoleSubsystem>();
	if (ConsoleSys)
	{
		NewSubsystem->RegisterCommands(ConsoleSys);
	}
}

void engine::Engine::InitSystems()
{
	Log::IsVerbose = launchArgs::GetArg("verbose").has_value();

	error::InitForThread("Main");
	error::OnErrorCallback = ErrorCallback;

	thread::IsMainThread = true;
	ThreadPool::AllocateDefaultThreadPool();
	Reflection::Init();
	resource::ScanForAssets();
}

void engine::Engine::ErrorCallback(string Error, string StackTrace)
{
#ifndef SERVER
	if (thread::IsMainThread)
	{
		subsystem::VideoSubsystem* VideoSys = Engine::GetSubsystem<subsystem::VideoSubsystem>();

		if (VideoSys)
		{
			delete VideoSys->MainWindow;
		}
	}
	kui::app::MessageBox(str::Format("%s!\nStack trace:\n%s", Error.c_str(), StackTrace.c_str()),
		"Engine error", kui::app::MessageBoxType::Error);
#endif
}
