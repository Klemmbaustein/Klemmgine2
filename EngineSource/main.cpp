#include "Engine/Engine.h"
#include <Engine/Subsystem/SceneSubsystem.h>
#include "Core/Types.h"

using namespace engine;
using engine::subsystem::SceneSubsystem;

int32 EngineMain(int argc, char** argv)
{
	Engine* Instance = Engine::Init();

	Instance->GetSubsystem<SceneSubsystem>()->LoadSceneAsync("Assets/Test");

	Instance->Run();
	return 0;
}