#include "EditorSubsystem.h"
#include <Engine/Subsystem/ConsoleSubsystem.h>
#include <Engine/Engine.h>
#include <Engine/Scene.h>
#include <Core/Platform/Platform.h>
#include <Engine/Debug/TimeLogger.h>
#include <Editor/Editor.h>
#include <Engine/Input.h>
#include <Editor/Settings/EditorSettings.h>
#include <Engine/Subsystem/SceneSubsystem.h>
#include <filesystem>

using namespace engine::editor;
using namespace engine;

bool editor::EditorSubsystem::Active = false;

static SerializedValue LastScene;
static string LastSceneName;

engine::editor::EditorSubsystem::EditorSubsystem()
	: subsystem::Subsystem("Editor", Log::LogColor::Yellow)
{
	Engine::IsPlaying = false;
	Engine::GameHasFocus = false;

	debug::TimeLogger UITime{ "Created editor UI", GetLogPrefixes() };

	UI = new EditorUI();
	Active = true;

	auto RemoteProject = editor::GetRemoteProjectName();
	if (RemoteProject)
	{
		std::filesystem::create_directories(GetEditorPath() + "/Remote/" + *RemoteProject + "/");
	}
	else
	{
		if (!std::filesystem::exists(".editor/"))
		{
			platform::CreateHiddenDirectory(".editor/");
		}
	}

	Settings::GetInstance()->Graphics.Apply();
}

void engine::editor::EditorSubsystem::RegisterCommands(ConsoleSubsystem* System)
{
	System->AddCommand(console::Command{
		.Name = "ed.run",
		.Args = {},
		.OnCalled = [](const console::Command::CallContext& ctx) {
			if (editor::IsActive())
			{
				Engine::GetSubsystem<EditorSubsystem>()->StartProject();
			}
		} });
	System->AddCommand(console::Command{
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

	auto console = Engine::GetSubsystem<ConsoleSubsystem>();

	if (console)
	{
		console->RemoveCommand("ed.edit");
		console->RemoveCommand("ed.run");
	}
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

	Engine::GameHasFocus = true;
	//Unload();

	bool ClearConsole = Settings::GetInstance()->Console.GetSetting("clearLogWhenGameStarts", false).GetBool();

	if (ClearConsole)
	{
		Log::Clear();
	}

	input::ShowMouseCursor = false;
	LastScene = Scene::GetMain()->Serialize();
	LastSceneName = Scene::GetMain()->Name;
	Scene::GetMain()->ReloadObjects(nullptr, [] {
		Engine::IsPlaying = true;
	});
	EditorUI::SetStatusMessage("Running game", editor::EditorUI::StatusType::Info);
}

void engine::editor::EditorSubsystem::StopProject()
{
	if (LastScene.GetType() != SerializedData::DataType::Null)
	{
		if (Scene::GetMain()->Name != LastSceneName)
		{
			Engine::IsPlaying = false;
			SceneSubsystem::Current->LoadSceneAsync(LastSceneName);
		}
		Scene::GetMain()->ReloadObjects(&LastScene, [] {
			Engine::IsPlaying = false;
		});
	}
	EditorUI::SetStatusMessage("Stopped game", editor::EditorUI::StatusType::Info);
}
