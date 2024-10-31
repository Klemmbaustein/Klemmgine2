#include "WorkingDirectory.h"
#include <filesystem>

void engine::internal::AdjustWorkingDirectory()
{
	std::filesystem::path CurrentDir = std::filesystem::current_path();

	if (std::filesystem::exists("Engine/"))
	{
		return;
	}

	while (true)
	{
		std::filesystem::path NewPath = CurrentDir / "../";
		if (!std::filesystem::exists(NewPath))
		{
			break;
		}

		if (std::filesystem::exists(NewPath / "Engine/"))
		{
			std::filesystem::current_path(NewPath);
			return;
		}
		CurrentDir = NewPath;
	}
}
