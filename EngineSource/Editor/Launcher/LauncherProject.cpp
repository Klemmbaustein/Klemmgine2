#include "LauncherProject.h"
#include <Editor/Editor.h>
#include <Core/File/JsonSerializer.h>
#include <Core/Log.h>

using namespace engine::editor::launcher;
using namespace engine;

std::vector<LauncherProject> engine::editor::launcher::LauncherProject::GetProjects()
{
		std::vector<LauncherProject> OutProjects;
	try
	{

		string FilePath = GetEditorPath() + "/projects.json";

		auto File = JsonSerializer::FromFile(FilePath);

		for (auto& i : File.GetArray())
		{
			OutProjects.push_back(LauncherProject{ i.At("name").GetString(), i.At("path").GetString() });
		}

	}
	catch (SerializeException& e)
	{
		Log::Note(e.what());
	}
	return OutProjects;
}

void engine::editor::launcher::LauncherProject::SaveProjects(std::vector<LauncherProject> Projects)
{
	SerializedValue val = std::vector<SerializedValue>();

	string FilePath = GetEditorPath() + "/projects.json";

	for (auto& i : Projects)
	{
		val.GetArray().push_back(SerializedValue({
			SerializedData("name", i.Name),
			SerializedData("path", i.Path)
			}));
	}
	JsonSerializer::ToFile(val, GetEditorPath() + "/projects.json", JsonSerializer::WriteOptions(true));
}
