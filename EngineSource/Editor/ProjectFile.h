#pragma once
#include <Core/Types.h>
#include <Core/File/SerializedData.h>

namespace engine::editor
{
	class ProjectFile : public ISerializable
	{
	public:
		ProjectFile(string Path);

		// Inherited via ISerializable
		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;

		void Save(string ToPath);

		string Name;
		string EngineVersion;
	};
}