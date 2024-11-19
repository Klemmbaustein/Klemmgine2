#include "Engine/Engine.h"
#include "Engine/Scene.h"
#include "Engine/Objects/MeshObject.h"
#include "Engine/File/TextSerializer.h"
#include <iostream>
#include <Engine/Subsystem/SceneSubsystem.h>
#include <Engine/Subsystem/ConsoleSubsystem.h>

int32 EngineMain(int argc, char** argv)
{
	using namespace engine;

	Engine* Instance = Engine::Init();

	subsystem::SceneSubsystem::Current->LoadSceneAsync("Assets/test.kscn");

	Instance->Run();
	
	return 0;
}