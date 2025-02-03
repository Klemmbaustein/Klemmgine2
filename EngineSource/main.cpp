#include "Engine/Engine.h"
#include <Engine/Subsystem/SceneSubsystem.h>
#include <iostream>
#include <Engine/Objects/MeshObject.h>
#include <Core/Error/EngineError.h>

int32 EngineMain(int argc, char** argv)
{
	using namespace engine;

	Engine* Instance = Engine::Init();
	subsystem::SceneSubsystem::Current->LoadSceneAsync("Assets/Test");

	Instance->Run();
	return 0;
}