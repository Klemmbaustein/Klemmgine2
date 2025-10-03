#pragma once
#include "AssetListProvider.h"

namespace engine::editor
{
	class FileAssetListProvider : public AssetListProvider
	{
	public:
		virtual ~FileAssetListProvider() = default;
		virtual std::vector<AssetFile> GetFiles(string Path) override;
	};

}