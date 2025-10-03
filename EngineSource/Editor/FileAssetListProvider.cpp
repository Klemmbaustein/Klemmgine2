#include "FileAssetListProvider.h"
#include <filesystem>

using namespace engine;

std::vector<editor::AssetFile> editor::FileAssetListProvider::GetFiles(string Path)
{
	if (!std::filesystem::exists("Assets/"))
	{
		std::filesystem::create_directories("Assets/");
	}

	if (!std::filesystem::exists(Path))
	{
		return {};
	}

	std::vector<AssetFile> Files;

	for (auto& i : std::filesystem::directory_iterator(Path))
	{
		Files.push_back(AssetFile{
			.Path = i.path().string(),
			.IsDirectory = i.is_directory(),
			});
	}

	return Files;
}
