#ifdef EDITOR
#include "EditorSubsystem.h"

bool engine::subsystem::EditorSubsystem::Active = false;

engine::subsystem::EditorSubsystem::EditorSubsystem()
	: ISubsystem("Editor", Log::LogColor::Yellow)
{
	UI = new editor::EditorUI();
	Active = true;
}
engine::subsystem::EditorSubsystem::~EditorSubsystem()
{
	delete UI;
	Active = false;
}
void engine::subsystem::EditorSubsystem::Update()
{
	UI->Update();
}
#endif