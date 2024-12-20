#include "Assets.h"
#include <filesystem>

std::map<engine::string, engine::string> engine::editor::assets::LoadedAssets;

void engine::editor::assets::ScanForAssets()
{
	for (const auto& i : std::filesystem::recursive_directory_iterator("Assets/"))
	{
		if (i.is_regular_file())
			LoadedAssets.insert({ i.path().filename().string(), str::ReplaceChar(i.path().string(), '\\', '/')});
	}
}
