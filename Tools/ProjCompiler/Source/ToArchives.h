#pragma once
#include <filesystem>
#include <set>
#include <vector>
#include <functional>
#include <Core/Types.h>

namespace fs = std::filesystem;

namespace engine::build
{
	struct AssetDependency
	{
		fs::path File;

		std::set<fs::path> DependentScenes;
	};
	struct ArchiveInfo
	{
		string Name;
		std::set<fs::path> Files;
		std::set<fs::path> DependentScenes;
	};

	std::vector<ArchiveInfo> GetBuildArchives(fs::path AssetsPath);
	std::set<fs::path> GetFileDependencies(fs::path FilePath, std::function<fs::path(string)> GetFileFromName);
}