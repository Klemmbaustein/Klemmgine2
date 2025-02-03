#ifdef EDITOR
#include "EditorSubsystem.h"
#include "ConsoleSubsystem.h"
#include <Engine/Engine.h>
#include <Engine/Scene.h>
#include <kui/Timer.h>

bool engine::subsystem::EditorSubsystem::Active = false;

static engine::SerializedValue LastScene;

engine::subsystem::EditorSubsystem::EditorSubsystem()
	: Subsystem("Editor", Log::LogColor::Yellow)
{
	if (Scene::GetMain())
	{
		if (LastScene.GetType() != SerializedData::DataType::Null)
		{
			Scene::GetMain()->ReloadObjects(&LastScene);
		}

	}
	kui::Timer t;

	UI = new editor::EditorUI();
	Active = true;

	Engine::GetSubsystem<ConsoleSubsystem>()->AddCommand(console::Command{
		.Name = "unload_editor",
		.Args = {},
		.OnCalled = [this](const console::Command::CallContext& ctx) {
			StartProject();
		}
		});

	Print(str::Format("Loaded editor UI (in %ims)", int(t.Get() * 1000.0f)), LogType::Note);
}

engine::subsystem::EditorSubsystem::~EditorSubsystem()
{
	Active = false;
	delete UI;
	Engine::GetSubsystem<ConsoleSubsystem>()->RemoveCommand("unload_editor");
}

void engine::subsystem::EditorSubsystem::Update()
{
	UI->Update();
}

void engine::subsystem::EditorSubsystem::StartProject()
{
	if (!Scene::GetMain())
	{
		editor::EditorUI::SetStatusMessage("Cannot start project, no scene loaded!", editor::EditorUI::StatusType::Error);
		return;
	}

	Active = false;
	Unload();
	
	LastScene = Scene::GetMain()->Serialize();
	Scene::GetMain()->ReloadObjects(nullptr);
}
#endif