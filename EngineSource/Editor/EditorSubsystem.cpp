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

bool editor::EditorSubsystem::Active = false;

static SerializedValue LastScene;

engine::editor::EditorSubsystem::EditorSubsystem()
	: subsystem::Subsystem("Editor", Log::LogColor::Yellow)
{
	Engine::IsPlaying = false;
	Engine::GameHasFocus = false;

	debug::TimeLogger UITime{ "Created editor UI", GetLogPrefixes() };

	UI = new EditorUI();
	Active = true;

	if (!std::filesystem::exists(".editor/"))
	{
		platform::CreateHiddenDirectory(".editor/");
	}

	Settings::GetInstance()->Graphics.Apply();
}

void engine::editor::EditorSubsystem::RegisterCommands(ConsoleSubsystem* System)
{
	Engine::GetSubsystem<ConsoleSubsystem>()->AddCommand(console::Command{
		.Name = "ed.run",
		.Args = {},
		.OnCalled = [](const console::Command::CallContext& ctx) {
			if (editor::IsActive())
			{
				Engine::GetSubsystem<EditorSubsystem>()->StartProject();
			}
		} });
	Engine::GetSubsystem<ConsoleSubsystem>()->AddCommand(console::Command{
		.Name = "ed.edit",
		.Args = {},
		.OnCalled = [this](const console::Command::CallContext& ctx) {
			if (!editor::IsActive())
			{
				Engine::Instance->LoadSubsystem(new EditorSubsystem());
			}
		} });
}

engine::editor::EditorSubsystem::~EditorSubsystem()
{
	Active = false;
	delete UI;
	Engine::GetSubsystem<ConsoleSubsystem>()->RemoveCommand("unload_editor");
}

void engine::editor::EditorSubsystem::Update()
{
	UI->Update();
}

void engine::editor::EditorSubsystem::StartProject()
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

void engine::editor::EditorSubsystem::StopProject()
{
	Engine::IsPlaying = false;
	if (LastScene.GetType() != SerializedData::DataType::Null)
	{
		Scene::GetMain()->ReloadObjects(&LastScene);
	}
	EditorUI::SetStatusMessage("Stopped game", editor::EditorUI::StatusType::Info);
}
