#include "Engine/Engine.h"
#include <Engine/Subsystem/SceneSubsystem.h>

int32 EngineMain(int argc, char** argv)
{
	using namespace engine;

	Engine* Instance = Engine::Init();

	subsystem::SceneSubsystem::Current->LoadSceneAsync("Assets/Test.kts");

	Instance->Run();
	
	return 0;
}