#include "OpenLauncher.h"
#include "EditorLauncher.h"

void engine::editor::launcher::OpenLauncher()
{
	auto Launcher = engine::editor::launcher::EditorLauncher();

	Launcher.Run();
}
