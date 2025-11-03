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
		if (i.size() <= Path.size())
		{
			continue;
		}

		auto FirstPart = i.substr(0, Path.size());
		auto SecondPart = i.substr(Path.size());

		if (FirstPart != Path || SecondPart.empty())
		{
			continue;
		}

		// Last character is a slash, meaning it's a directory.
		if (*i.rbegin() == '/')
		{
			Files.push_back(AssetFile{
				.Path = i.substr(0, i.size() - 1),
				.IsDirectory = true,
				});
			continue;
		}

		if (SecondPart.find('/') != std::string::npos)
		{
			continue;
		}

		Files.push_back(AssetFile{
			.Path = i,
			.IsDirectory = false,
			});
	}

	return Files;
}

void engine::editor::ServerAssetsProvider::DeleteFile(string Path)
{
	Connection->SendMessage("deleteFile", SerializedValue({
		SerializedData("path", Path)
		}));
}

void engine::editor::ServerAssetsProvider::NewFile(string Path)
{
	Connection->SendMessage("newFile", SerializedValue({
		SerializedData("path", Path),
		}));
}

void engine::editor::ServerAssetsProvider::NewDirectory(string Path)
{
	Connection->SendMessage("newFile", SerializedValue({
		SerializedData("path", Path),
		SerializedData("isDirectory", true),
		}));
}

void engine::editor::ServerAssetsProvider::SaveToFile(string Path, IBinaryStream* Stream, size_t Length)
{
	Connection->SendFile(Path, Stream, Length);
}
