#include "ServerAssetsProvider.h"

using namespace engine::editor;

engine::editor::ServerAssetsProvider::ServerAssetsProvider(ServerConnection* Connection)
{
	this->Connection = Connection;
}

std::vector<AssetFile> engine::editor::ServerAssetsProvider::GetFiles(string Path)
{
	std::vector<AssetFile> Files;

	for (auto& i : Connection->Files)
	{
		Files.push_back(AssetFile{
			.Path = i,
			.IsDirectory = false,
			});
	}

	return Files;
}
