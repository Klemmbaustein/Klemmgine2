#include "Editor.h"
#include "EditorSubsystem.h"
using namespace engine;
using namespace engine::subsystem;

static string EditorPath = "Engine/";

const bool editor::IsActive()
{
	return EditorSubsystem::Active;
}

string engine::editor::GetEditorPath()
{
	return EditorPath;
}

void engine::editor::OpenEditorAt(string Path)
{
	EditorPath = Path;
}