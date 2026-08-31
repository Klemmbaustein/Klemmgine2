#include "ProjectFile.h"
#include <Core/File/JsonSerializer.h>
#include <Engine/Version.h>
#include <Engine/File/Resource.h>

using namespace engine;

engine::ProjectFile::ProjectFile(string Path)
{
	if (!resource::FileExists(Path))
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

engine::ProjectFile::ProjectFile()
{
}

SerializedValue engine::ProjectFile::Serialize()
{
	return SerializedValue({
		SerializedData("name", this->Name),
		SerializedData("engineVersion", VersionInfo::Get().GetShortName()),
		SerializedData("startupScene", this->StartupScene),
		SerializedData("useScriptJIT", this->UseScriptJIT)
		});
}

void engine::ProjectFile::DeSerialize(SerializedValue* From)
{
	this->Name = From->At("name").GetString();
	this->EngineVersion = From->At("engineVersion").GetString();
	this->StartupScene = From->At("startupScene").GetString();
	if (From->Contains("useScriptJIT"))
	{
		this->UseScriptJIT = From->At("useScriptJIT").GetBool();
	}
}

void engine::ProjectFile::Save(string ToPath)
{
	JsonSerializer::ToFile(Serialize(), ToPath, JsonSerializer::WriteOptions(true));
}
