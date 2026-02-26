#include "Engine.h"
#include "Version.h"
#include "Graphics/VideoSubsystem.h"
#include "Subsystem/InputSubsystem.h"
#include "Subsystem/ConsoleSubsystem.h"
#include "Subsystem/SceneSubsystem.h"
#include "Sound/SoundSubsystem.h"
#include "Plugins/PluginSubsystem.h"
#include "Script/ScriptSubsystem.h"
#include "MainThread.h"
#include "Core/ThreadPool.h"
#include "File/Resource.h"
#include "Core/Error/EngineError.h"
#include <kui/App.h>
#include <Engine/Debug/TimeLogger.h>
#include <Core/LaunchArgs.h>

#ifdef EDITOR
#include <Editor/EditorSubsystem.h>
#endif

using namespace engine;
using namespace engine::subsystem;

Engine* Engine::Instance = nullptr;
bool Engine::IsPlaying = true;
bool Engine::GameHasFocus = true;

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
	Log::Info(VersionInfo::Get().GetDisplayNameAndBuild());

	Instance = new Engine();

	Instance->InitSystems();

	Instance->LoadSubsystem(new ConsoleSubsystem());
	Instance->LoadSubsystem(new PluginSubsystem());
	Instance->LoadSubsystem(new VideoSubsystem());
	Instance->LoadSubsystem(new InputSubsystem());
#ifdef ENGINE_ENABLE_SOUND
	Instance->LoadSubsystem(new sound::SoundSubsystem());
#endif
	Instance->LoadSubsystem(new script::ScriptSubsystem());
	Instance->LoadSubsystem(new SceneSubsystem());

#ifdef EDITOR
	Instance->LoadSubsystem(new editor::EditorSubsystem());
#endif

	return Instance;
}

void Engine::Run()
{
	while (!ShouldQuit)
	{
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

	ThreadPool::FreeDefaultThreadPool();

	for (int64 i = LoadedSystems.size() - 1; i >= 0; i--)
	{
		delete LoadedSystems[i];
	}
	LoadedSystems.clear();

	delete this;
}

subsystem::Subsystem* Engine::LoadSubsystem(Subsystem* NewSubsystem)
{
	if (typeid(*NewSubsystem) == typeid(ConsoleSubsystem))
	{
		for (auto& i : LoadedSystems)
		{
			i->RegisterCommands(static_cast<ConsoleSubsystem*>(NewSubsystem));
		}
		return NewSubsystem;
	}

	ConsoleSubsystem* ConsoleSys = GetSubsystem<ConsoleSubsystem>();
	if (ConsoleSys)
	{
		NewSubsystem->RegisterCommands(ConsoleSys);
	}

	return NewSubsystem;
}

void engine::Engine::InitSystems()
{
	Log::IsVerbose = launchArgs::GetArg("verbose").has_value() ? true : Log::IsVerbose;

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
		VideoSubsystem* VideoSys = Engine::GetSubsystem<VideoSubsystem>();

		if (VideoSys)
		{
			delete VideoSys->MainWindow;
		}
	}

	string ErrorMessage = str::Format("%s!\nStack trace:\n%s", Error.c_str(), StackTrace.c_str());

	Log::Critical(ErrorMessage);

	kui::app::MessageBox(ErrorMessage,
		"Engine error", kui::app::MessageBoxType::Error);
#endif
}
