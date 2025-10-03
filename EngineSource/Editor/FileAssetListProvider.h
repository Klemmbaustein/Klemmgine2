#pragma once
#include "AssetListProvider.h"
#include <Core/Event.h>

namespace engine::editor
{
	class FileAssetListProvider : public AssetListProvider
	{
	public:
		virtual ~FileAssetListProvider() = default;
		std::vector<AssetFile> GetFiles(string Path) override;
		void DeleteFile(string Path) override;
		void NewFile(string Path) override;
		void NewDirectory(string Path) override;
		IBinaryStream* GetFileSaveStream(string Path) override;
	};
}