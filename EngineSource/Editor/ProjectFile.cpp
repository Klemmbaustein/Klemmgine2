#include "ProjectFile.h"
#include <Core/File/JsonSerializer.h>

using namespace engine;

engine::editor::ProjectFile::ProjectFile(string Path)
{
	auto JsonData = JsonSerializer::FromFile(Path);

	DeSerialize(&JsonData);
}

SerializedValue engine::editor::ProjectFile::Serialize()
{
	return SerializedValue({
		SerializedData("name", this->Name),
		SerializedData("engineVersion", this->EngineVersion)
		});
}

void engine::editor::ProjectFile::DeSerialize(SerializedValue* From)
{
	this->Name = From->At("name").GetString();
	this->EngineVersion = From->At("engineVersion").GetString();
}

void engine::editor::ProjectFile::Save(string ToPath)
{
	JsonSerializer::ToFile(Serialize(), ToPath, JsonSerializer::WriteOptions(true));
}
