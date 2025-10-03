#pragma once
#include <Editor/AssetListProvider.h>
#include <Editor/Server/ServerConnection.h>

namespace engine::editor
{
	class ServerAssetsProvider : public AssetListProvider
	{
	public:

		ServerAssetsProvider(ServerConnection* Connection);

		std::vector<AssetFile> GetFiles(string Path) override;

	private:
		ServerConnection* Connection;
	};
}