#include "SceneSubsystem.h"
#include "ConsoleSubsystem.h"
#include <Engine/MainThread.h>
#include <Engine/Engine.h>
#include <thread>

engine::subsystem::SceneSubsystem* engine::subsystem::SceneSubsystem::Current = nullptr;

engine::subsystem::SceneSubsystem::SceneSubsystem()
	: Subsystem("Scene", Log::LogColor::Green)
{
	Current = this;
	auto Console = Engine::GetSubsystem<ConsoleSubsystem>();

	if (Console)
	{
		Console->AddCommand(console::Command{
			.Name = "open",
			.Args = { console::Command::Argument{.Name = "targetScene", .Required = true}},
			.OnCalled = [this](const console::Command::CallContext& ctx) {
				LoadSceneAsync(ctx.ProvidedArguments[0]);
			}
			});
	}
}

engine::subsystem::SceneSubsystem::~SceneSubsystem()
{
	std::vector OldScenes = LoadedScenes;
	LoadedScenes.clear();
	for (Scene* scn : OldScenes)
	{
		delete scn;
	}
	Current = nullptr;
}

void engine::subsystem::SceneSubsystem::LoadSceneAsync(string SceneName)
{
	Print(str::Format("Loading scene asynchronously: %s", SceneName.c_str()), LogType::Info);

	std::thread([this, SceneName]() {
		LoadSceneThread(SceneName);
		}).detach();
}

void engine::subsystem::SceneSubsystem::Update()
{
	for (Scene* scn : LoadedScenes)
	{
		scn->Update();
	}
}

void engine::subsystem::SceneSubsystem::LoadSceneThread(string SceneName)
{
	Scene* New = new Scene(true);

	New->LoadAsync(SceneName);

	thread::ExecuteOnMainThread([this, SceneName, New]() {
		if (Main)
		{
			delete Main;
			Main = nullptr;
		}

		Print(str::Format("Finished loading scene: %s", SceneName.c_str()));
		New->LoadAsyncFinish();
		Main = New;
		});
}
