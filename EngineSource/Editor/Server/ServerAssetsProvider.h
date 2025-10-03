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
		void DeleteFile(string Path) override;
		void NewFile(string Path) override;
		void NewDirectory(string Path) override;
		IBinaryStream* GetFileSaveStream(string Path) override
		{
			return nullptr;
		}
		void SaveToFile(string Path, IBinaryStream* Stream, size_t Length) override;

	private:
		ServerConnection* Connection;
	};
}