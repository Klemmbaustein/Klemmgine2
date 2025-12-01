#include "Engine/Engine.h"
#include <Engine/Subsystem/SceneSubsystem.h>
#include "Core/Types.h"
#ifdef EDITOR
#include <Editor/Launcher/OpenLauncher.h>

using namespace engine;

int32 EngineMain(int argc, char** argv)
{
	editor::launcher::OpenLauncher();
	//Engine* Instance = Engine::Init();

	//Instance->GetSubsystem<SceneSubsystem>()->LoadSceneAsync("Assets/Test");

	//Instance->Run();
	return 0;
}
#else

using namespace engine;

int32 EngineMain(int argc, char** argv)
{
	Engine* Instance = Engine::Init();

	Instance->GetSubsystem<SceneSubsystem>()->LoadSceneAsync("Assets/Test");

	Instance->Run();
	return 0;
}

#endif