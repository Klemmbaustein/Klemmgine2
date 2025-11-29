#include "Engine/Engine.h"
#include <Engine/Subsystem/SceneSubsystem.h>
#include "Core/Types.h"
#include <Editor/Launcher/EditorLauncher.h>

using namespace engine;

int32 EngineMain(int argc, char** argv)
{
	engine::editor::launcher::EditorLauncher();

	Engine* Instance = Engine::Init();

	Instance->GetSubsystem<SceneSubsystem>()->LoadSceneAsync("Assets/Test");

	Instance->Run();
	return 0;
}