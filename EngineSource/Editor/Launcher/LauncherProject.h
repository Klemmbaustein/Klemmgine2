#pragma once
#include <Core/Types.h>

namespace engine::editor::launcher
{
	struct LauncherProject
	{
		string Name;
		string Path;

		static std::vector<LauncherProject> GetProjects();
	};
}