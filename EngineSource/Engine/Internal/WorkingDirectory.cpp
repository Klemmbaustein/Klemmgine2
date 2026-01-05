#include "WorkingDirectory.h"
#include <filesystem>

#ifdef EDITOR
constexpr const char* TARGET_DIR = "Engine";
#else
constexpr const char* TARGET_DIR = "Assets";
#endif

void engine::internal::AdjustWorkingDirectory()
{
	std::filesystem::path CurrentDir = std::filesystem::current_path();

	if (std::filesystem::exists(TARGET_DIR))
	{
		return;
	}

	while (true)
	{
		if (std::filesystem::exists(CurrentDir / TARGET_DIR))
		{
			std::filesystem::current_path(CurrentDir);
			return;
		}

		std::filesystem::path NewPath = CurrentDir / "..";
		if (!std::filesystem::exists(NewPath) || std::filesystem::canonical(NewPath) == "/")
		{
			break;
		}
		CurrentDir = NewPath;
	}
}
