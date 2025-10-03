#pragma once
#include <Core/Types.h>
#include <vector>

namespace engine::editor
{
	struct AssetFile
	{
		string Path;
		bool IsDirectory;
	};

	class AssetListProvider
	{
	public:
		virtual ~AssetListProvider() = default;
		virtual std::vector<AssetFile> GetFiles(string Path) = 0;
	};
}