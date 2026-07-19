#include "OpenLauncher.h"
#include "EditorLauncher.h"
#include <Core/LaunchArgs.h>
#include <Engine/Engine.h>
#include <Editor/Editor.h>
#include <filesystem>

using namespace engine;

static void OpenEditor(std::optional<std::filesystem::path> NewPath)
{
	auto OldPath = std::filesystem::current_path();
	editor::OpenEditorAt(std::filesystem::canonical("./Engine/").string());

	if (NewPath)
	{
		std::filesystem::current_path(*NewPath);
	}

	auto Engine = Engine::Init();
	Engine->Run();

	if (editor::launcher::EditorLauncher::ReOpenLauncher)
	{
		if (NewPath)
		{
			std::filesystem::current_path(OldPath);
		}

		Log::Clear();
		auto Launcher = engine::editor::launcher::EditorLauncher();

		Launcher.Run();
		return;
	}
}

void engine::editor::launcher::OpenLauncher()
{
	if (launchArgs::GetArg("here"))
	{
		OpenEditor(std::nullopt);
		return;
	}
	if (launchArgs::GetArg("path"))
	{
		OpenEditor(launchArgs::GetArg("path").value()[0].AsString());
		return;
	}

	auto Launcher = engine::editor::launcher::EditorLauncher();

	Launcher.Run();
}
