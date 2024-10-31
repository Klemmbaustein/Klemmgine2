#ifdef EDITOR
#include "EditorSubsystem.h"
#include "VideoSubsystem.h"
#include "Engine/Engine.h"

engine::subsystem::EditorSubsystem::EditorSubsystem()
	: ISubsystem("Editor")
{
	UI = new editor::EditorUI();
}
void engine::subsystem::EditorSubsystem::Update()
{
	UI->Update();
}
#endif