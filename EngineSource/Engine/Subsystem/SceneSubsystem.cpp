#include "SceneSubsystem.h"
#include "ConsoleSubsystem.h"
#include <Engine/MainThread.h>
#include <Engine/Engine.h>
#include <Core/ThreadPool.h>

engine::SceneSubsystem* engine::SceneSubsystem::Current = nullptr;

engine::SceneSubsystem::SceneSubsystem()
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

engine::SceneSubsystem::~SceneSubsystem()
{
	std::vector OldScenes = LoadedScenes;
	LoadedScenes.clear();
	for (Scene* scn : OldScenes)
	{
		delete scn;
	}
	Current = nullptr;
}

void engine::SceneSubsystem::LoadSceneAsync(string SceneName)
{
	Print(str::Format("Loading scene asynchronously: %s", SceneName.c_str()), LogType::Info);

	ThreadPool::Main()->AddJob([this, SceneName]() {
		LoadSceneThread(SceneName);
	});
}

void engine::SceneSubsystem::Update()
{
	for (Scene* scn : LoadedScenes)
	{
		scn->Update();
	}

	for (auto& i : ToDelete)
	{
		delete i;
	}
}

void engine::SceneSubsystem::SetAsMain(Scene* Target)
{
	if (Main == Target)
	{
		return;
	}

	Print(str::Format("Setting new scene as main: %s", Target->Name.c_str()));

	if (Main)
	{
		ToDelete.push_back(Main);
	}
	Main = Target;
}

void engine::SceneSubsystem::LoadSceneThread(string SceneName)
{
	IsLoading = true;
	Scene* New = new Scene(true);

	New->LoadAsync(SceneName);

	thread::ExecuteOnMainThread([this, SceneName, New]() {
		if (Main)
		{
			delete Main;
			Main = nullptr;
		}
		IsLoading = false;

		Print(str::Format("Finished loading scene: %s", SceneName.c_str()));
		New->LoadAsyncFinish();
		Main = New;
		});
}
