#include "Engine/Engine.h"
#include <Engine/Subsystem/SceneSubsystem.h>
#include <Engine/Graphics/VideoSubsystem.h>
#include "Core/Types.h"
#include <Engine/ProjectFile.h>
#include <filesystem>
#include <Core/LaunchArgs.h>

using namespace engine;

static inline int32 LaunchEngine(int argc, char** argv)
{
	Engine* Instance = Engine::Init();

	if (std::filesystem::exists("Assets/project.json"))
	{
		auto proj = ProjectFile("Assets/project.json");
		Instance->GetSubsystem<SceneSubsystem>()->LoadSceneAsync(proj.StartupScene);
		Instance->GetSubsystem<VideoSubsystem>()->MainWindow->SetTitle(proj.Name);
	}
	if (std::filesystem::exists("project.json"))
	{
		auto proj = ProjectFile("project.json");
		Instance->GetSubsystem<SceneSubsystem>()->LoadSceneAsync(proj.StartupScene);
		Instance->GetSubsystem<VideoSubsystem>()->MainWindow->SetTitle(proj.Name);
	}

	Instance->Run();
	return 0;
}


#ifdef EDITOR
#include <Editor/Launcher/OpenLauncher.h>

int32 EngineMain(int argc, char** argv)
{
	if (launchArgs::GetArg("noEditor"))
	{
		LaunchEngine(argc, argv);
	}
	else
	{
		editor::launcher::OpenLauncher();
	}
	return 0;
}

#else

int32 EngineMain(int argc, char** argv)
{
	return LaunchEngine(argc, argv);
}

#endif