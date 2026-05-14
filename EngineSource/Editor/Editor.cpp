#include "Editor.h"
#include "EditorSubsystem.h"
#include <Engine/File/Resource.h>
using namespace engine;
using namespace engine::subsystem;

static string EditorPath = "Engine";
static std::optional<string> RemoteProject = std::nullopt;

const bool editor::IsActive()
{
	return EditorSubsystem::Active;
}

string engine::editor::GetEditorPath()
{
	return EditorPath;
}

std::optional<string> engine::editor::GetRemoteProjectName()
{
	return RemoteProject;
}

void engine::editor::SetRemoteProject(string Path)
{
	resource::AllowLocalFiles = false;
	RemoteProject = str::ReplaceChar(str::ReplaceChar(Path, ':', '-'), '/', '-');
}

void engine::editor::ClearRemoteProject()
{
	resource::AllowLocalFiles = true;
	RemoteProject = std::nullopt;
}

void engine::editor::OpenEditorAt(string Path)
{
#ifdef WINDOWS
	EditorPath = str::ReplaceChar(Path, '\\', '/');
#else
	EditorPath = Path;
#endif
}