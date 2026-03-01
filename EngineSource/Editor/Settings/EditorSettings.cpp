#include "EditorSettings.h"
#include <Core/File/JsonSerializer.h>
#include <Editor/Editor.h>
#include <Core/Log.h>
#include <filesystem>

using namespace engine;
using namespace engine::editor;

engine::editor::Settings::Settings()
{
	AddCategory(&Interface);
	AddCategory(&Script);
	AddCategory(&Console);
	AddCategory(&Graphics);

	try
	{
		auto Settings = JsonSerializer::FromFile(GetSettingsPath());

		DeSerialize(&Settings);
	}
	catch (SerializeException& e)
	{
		Log::Warn(str::Format("%s\nUsing default editor settings", e.what()));
	}
}

engine::editor::Settings::~Settings()
{
}

void engine::editor::Settings::Save()
{
	std::filesystem::create_directory(GetEditorPath() + "/Config/);
	JsonSerializer::ToFile(Serialize(), GetSettingsPath(), JsonSerializer::WriteOptions(true));
}

Settings* engine::editor::Settings::GetInstance()
{
	if (!Instance)
	{
		Instance = new Settings();
	}

	return Instance;
}

SerializedValue engine::editor::Settings::Serialize()
{
	std::vector<SerializedData> out;
	for (auto& i : this->Categories)
	{
		out.push_back(SerializedData(i->Name, i->Serialize()));
	}

	return out;
}

void engine::editor::Settings::DeSerialize(SerializedValue* From)
{
	for (auto& i : this->Categories)
	{
		if (From->Contains(i->Name))
		{
			i->DeSerialize(&From->At(i->Name));
		}
	}
}

void engine::editor::Settings::AddCategory(SettingsCategory* NewCategory)
{
	Categories.push_back(NewCategory);
}

string engine::editor::Settings::GetSettingsPath()
{
	return GetEditorPath() + "/Config/settings.json";
}
