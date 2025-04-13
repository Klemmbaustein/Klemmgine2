#if WINDOWS
#pragma once
#include "BuildProject.h"

namespace engine::editor
{
	void BuildProjectLinux(BuildOptions Options);
	void BuildProjectWinX64(BuildOptions Options);
}
#endif