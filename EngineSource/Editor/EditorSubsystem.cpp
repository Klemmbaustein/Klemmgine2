#include "EditorSubsystem.h"
#include <Engine/Subsystem/ConsoleSubsystem.h>
#include <Engine/Engine.h>
#include <Engine/Scene.h>
#include <Core/Platform/Platform.h>
#include <Engine/Debug/TimeLogger.h>
#include <Editor/Editor.h>
#include <Engine/Input.h>
#include <Editor/Settings/EditorSettings.h>
#include <filesystem>

using namespace engine::editor;
using namespace engine;

bool subsystem::EditorSubsystem::Active = false;

static SerializedValue LastScene;

engine::subsystem::EditorSubsystem::EditorSubsystem()
	: Subsystem("Editor", Log::LogColor::Yellow)
{
	THIS_SUBSYSTEM_DEPENDS_ON(ConsoleSubsystem);

	debug::TimeLogger UITime{ "Created editor UI", GetLogPrefixes() };

	UI = new editor::EditorUI();
	Active = true;

	Engine::GetSubsystem<ConsoleSubsystem>()->AddCommand(console::Command{
		.Name = "ed.run",
		.Args = {},
		.OnCalled = [](const console::Command::CallContext& ctx) {
			if (editor::IsActive())
			{
				Engine::GetSubsystem<EditorSubsystem>()->StartProject();
			}
		}
		});
	Engine::GetSubsystem<ConsoleSubsystem>()->AddCommand(console::Command{
	.Name = "ed.edit",
	.Args = {},
	.OnCalled = [this](const console::Command::CallContext& ctx) {
			if (!editor::IsActive())
			{
				Engine::Instance->LoadSubsystem(new EditorSubsystem());
			}
		}
		});

	if (!std::filesystem::exists(".editor/"))
	{
		platform::CreateHiddenDirectory(".editor/");
	}

	Settings::GetInstance()->Graphics.Apply();
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
		EditorUI::SetStatusMessage("Cannot start project, no scene loaded!", editor::EditorUI::StatusType::Error);
		return;
	}

	Engine::IsPlaying = true;
	//Unload();

	input::ShowMouseCursor = false;
	LastScene = Scene::GetMain()->Serialize();
	Scene::GetMain()->ReloadObjects(nullptr);
	EditorUI::SetStatusMessage("Running game", editor::EditorUI::StatusType::Info);
}

void engine::subsystem::EditorSubsystem::StopProject()
{
	Engine::IsPlaying = false;
	if (LastScene.GetType() != SerializedData::DataType::Null)
	{
		Scene::GetMain()->ReloadObjects(&LastScene);
	}
	EditorUI::SetStatusMessage("Stopped game", editor::EditorUI::StatusType::Info);
}
