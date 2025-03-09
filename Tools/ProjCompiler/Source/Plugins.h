#pragma once
#include <filesystem>

namespace fs = std::filesystem;

namespace engine::build
{
	void CopyPluginFiles(fs::path BinaryPath, fs::path OutPath);
}