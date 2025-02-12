#include "WorkingDirectory.h"
#include <filesystem>

void engine::internal::AdjustWorkingDirectory()
{
	std::filesystem::path CurrentDir = std::filesystem::current_path();

	if (std::filesystem::exists("Assets"))
	{
		return;
	}

	while (true)
	{
		if (std::filesystem::exists(CurrentDir / "Assets"))
		{
			std::filesystem::current_path(CurrentDir);
			return;
		}

		std::filesystem::path NewPath = CurrentDir / "..";
		if (!std::filesystem::exists(NewPath) || !NewPath.has_root_name())
		{
			break;
		}
		CurrentDir = NewPath;
	}
}
