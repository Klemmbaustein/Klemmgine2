#pragma once
#include <Core/Types.h>
#include <map>
#include <Core/File/BinaryStream.h>

namespace engine::resource
{
	class ResourceSource
	{
	public:
		virtual ~ResourceSource() = default;
		virtual bool FileExists(string Path) = 0;
		virtual ReadOnlyBufferStream* GetFile(string Path) = 0;
		virtual std::map<string, string> GetFiles() = 0;
		virtual void LoadSceneFiles(string ScenePath) {};
	};

	[[nodiscard]]
	string GetTextFile(string EnginePath);
	[[nodiscard]]
	string ConvertFilePath(string EnginePath, bool AllowFile = false);

	[[nodiscard]]
	bool FileExists(string EnginePath);

	[[nodiscard]]
	ReadOnlyBufferStream* GetBinaryFile(string EnginePath);

	extern std::map<string, string> LoadedAssets;

	void LoadSceneFiles(string ScenePath);
	void ScanForAssets();
	void AddResourceSource(ResourceSource* Source);
	void RemoveResourceSource(ResourceSource* Source);
}