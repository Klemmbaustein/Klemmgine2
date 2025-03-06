#pragma once
#include <Core/Types.h>
#include <map>
#include <Core/File/BinaryStream.h>

namespace engine::resource
{
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
}