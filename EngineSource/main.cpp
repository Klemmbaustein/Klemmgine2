#include "Engine/Engine.h"
#include <Engine/Subsystem/SceneSubsystem.h>
#include "Core/Types.h"

int32 EngineMain(int argc, char** argv)
{
	using namespace engine;
	using engine::subsystem::SceneSubsystem;

	Engine* Instance = Engine::Init();
	Instance->GetSubsystem<SceneSubsystem>()->LoadSceneAsync("Assets/Test");

	Instance->Run();
	return 0;
}