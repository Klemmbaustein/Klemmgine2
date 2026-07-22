#pragma once
#include "AssetListProvider.h"
#include <Core/Platform/FileWatcher.h>

namespace engine::editor
{
	class FileAssetListProvider : public AssetListProvider
	{
	public:
		~FileAssetListProvider() override;

		std::vector<AssetFile> GetFiles(string Path) override;
		void DeleteFile(string Path) override;
		void NewFile(string Path) override;
		void NewDirectory(string Path) override;
		IBinaryStream* GetFileSaveStream(string Path) override;

		void Update() override;

	private:
		FileWatcher* Watcher = nullptr;
	};
}