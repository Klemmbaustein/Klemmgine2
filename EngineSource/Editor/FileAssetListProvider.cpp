#include "FileAssetListProvider.h"
#include <filesystem>
#include <fstream>

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

void engine::editor::FileAssetListProvider::DeleteFile(string Path)
{
	std::filesystem::remove_all(Path);
	OnChanged.Invoke();
}

void engine::editor::FileAssetListProvider::NewFile(string Path)
{
	std::ofstream File = std::ofstream(Path);
	File.close();
	OnChanged.Invoke();
}

void engine::editor::FileAssetListProvider::NewDirectory(string Path)
{
	std::filesystem::create_directories(Path);
	OnChanged.Invoke();
}

IBinaryStream* engine::editor::FileAssetListProvider::GetFileSaveStream(string Path)
{
	return new FileStream(Path, false);
}
