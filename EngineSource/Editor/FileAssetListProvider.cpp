#include "FileAssetListProvider.h"
#include <filesystem>
#include <fstream>
#include <Core/Log.h>

using namespace engine;

engine::editor::FileAssetListProvider::~FileAssetListProvider()
{
	delete Watcher;
}

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

	Watcher = new FileWatcher("Assets/");
	Watcher->OnFileChanged.Add(this, [this](FileChange Change) {
		if (Change.Type != FileChangeType::Modified)
		{
			this->OnChanged.Invoke();
		}
		this->OnModified.Invoke(Change.FilePath);
	});

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
	try
	{
		std::filesystem::remove_all(Path);
		Watcher->OnFileChanged.Invoke(FileChange{
			.Type = FileChangeType::Removed,
			.FilePath = Path,
			});
	}
	catch (std::filesystem::filesystem_error& e)
	{
		Log::Error(e.what());
	}
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

void engine::editor::FileAssetListProvider::Update()
{
	if (Watcher)
	{
		Watcher->Update();
	}
}
