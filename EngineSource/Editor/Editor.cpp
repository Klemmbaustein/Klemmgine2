#include "Editor.h"
#include "EditorSubsystem.h"
using namespace engine;
using namespace engine::subsystem;

static string EditorPath = "Engine";

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
#ifdef WINDOWS
	EditorPath = str::ReplaceChar(Path, '\\', '/');
#else
	EditorPath = Path;
#endif
}