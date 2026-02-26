#include "ProjectFile.h"
#include <Core/File/JsonSerializer.h>
#include <Engine/Version.h>
#include <filesystem>

using namespace engine;

engine::ProjectFile::ProjectFile(string Path)
{
	if (!std::filesystem::exists(Path))
	{
		Save(Path);
		return;
	}

	try
	{
		auto JsonData = JsonSerializer::FromFile(Path);

		DeSerialize(&JsonData);
	}
	catch (SerializeException& e)
	{
		Save(Path);
	}
}

SerializedValue engine::ProjectFile::Serialize()
{
	return SerializedValue({
		SerializedData("name", this->Name),
		SerializedData("engineVersion", VersionInfo::Get().GetShortName()),
		SerializedData("startupScene", this->StartupScene),
		});
}

void engine::ProjectFile::DeSerialize(SerializedValue* From)
{
	this->Name = From->At("name").GetString();
	this->EngineVersion = From->At("engineVersion").GetString();
	this->StartupScene = From->At("startupScene").GetString();
}

void engine::ProjectFile::Save(string ToPath)
{
	JsonSerializer::ToFile(Serialize(), ToPath, JsonSerializer::WriteOptions(true));
}
