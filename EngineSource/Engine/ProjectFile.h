#pragma once
#include <Core/Types.h>
#include <Core/File/SerializedData.h>

namespace engine
{
	class ProjectFile : public Serializable
	{
	public:
		ProjectFile(string Path);
		ProjectFile();

		// Inherited via Serializable
		SerializedValue Serialize() override;
		void DeSerialize(SerializedValue* From) override;

		void Save(string ToPath);

		string Name = "Untitled";
		string EngineVersion;
		string StartupScene;
		bool UseScriptJIT = false;
	};
}