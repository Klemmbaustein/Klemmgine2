#include "SceneSubsystem.h"
#include <Engine/MainThread.h>
#include <thread>

engine::subsystem::SceneSubsystem* engine::subsystem::SceneSubsystem::Current = nullptr;

engine::subsystem::SceneSubsystem::SceneSubsystem()
	: ISubsystem("Scene", Log::LogColor::Green)
{
	Current = this;
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
		Print(str::Format("Finished loading scene: %s", SceneName.c_str()));
		New->LoadAsyncFinish();
		Main = New;
		});
}
