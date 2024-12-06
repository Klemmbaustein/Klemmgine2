#pragma once
#include <Engine/Types.h>
#include <map>

namespace engine::editor::assets
{
	extern std::map<string, string> LoadedAssets;

	void ScanForAssets();
}