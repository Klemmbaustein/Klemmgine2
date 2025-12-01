#include "LauncherProject.h"
#include <Editor/Editor.h>
#include <Core/File/JsonSerializer.h>
#include <Core/Log.h>

using namespace engine::editor::launcher;
using namespace engine;

std::vector<LauncherProject> engine::editor::launcher::LauncherProject::GetProjects()
{
	std::vector<LauncherProject> OutProjects;

	string FilePath = GetEditorPath() + "/projects.json";

	auto File = JsonSerializer::FromFile(FilePath);

	for (auto& i : File.GetArray())
	{
		OutProjects.push_back(LauncherProject{ i.At("name").GetString(), i.At("path").GetString() });
	}

	return OutProjects;
}