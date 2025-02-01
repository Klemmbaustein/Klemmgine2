#include "WorkingDirectory.h"
#include <Core/Log.h>
#include <filesystem>

void engine::internal::AdjustWorkingDirectory()
{
	std::filesystem::path CurrentDir = std::filesystem::current_path();

	if (std::filesystem::exists("Assets/"))
	{
		Log::Info(std::filesystem::current_path().string());
		return;
	}

	while (true)
	{
		if (std::filesystem::exists(CurrentDir / "Assets/"))
		{
			std::filesystem::current_path(CurrentDir);
			return;
		}

		std::filesystem::path NewPath = CurrentDir / "../";
		if (!std::filesystem::exists(NewPath))
		{
			break;
		}

		CurrentDir = NewPath;
	}
}
