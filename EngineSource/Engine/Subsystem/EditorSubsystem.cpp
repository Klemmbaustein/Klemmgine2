#ifdef EDITOR
#include "EditorSubsystem.h"
#include "ConsoleSubsystem.h"
#include <Engine/Engine.h>

bool engine::subsystem::EditorSubsystem::Active = false;

engine::subsystem::EditorSubsystem::EditorSubsystem()
	: Subsystem("Editor", Log::LogColor::Yellow)
{
	UI = new editor::EditorUI();
	Active = true;

	Engine::GetSubsystem<ConsoleSubsystem>()->AddCommand(console::Command{
		.Name = "unload_editor",
		.Args = {},
		.OnCalled = [this](const console::Command::CallContext& ctx) {
			Unload();
		}
		});
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
#endif