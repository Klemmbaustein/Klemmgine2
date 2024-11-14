#include "Engine/Engine.h"
#include "Engine/Scene.h"
#include "Engine/Objects/MeshObject.h"
#include "Engine/File/TextSerializer.h"
#include <iostream>

int32 EngineMain(int argc, char** argv)
{
	using namespace engine;

	Engine* Instance = Engine::Init();

	auto Object = Scene::GetMain()->CreateObject<MeshObject>();
	Object->Name = "Test Object";

	auto Object2 = Scene::GetMain()->CreateObject<MeshObject>();
	Object->Name = "Test Object2";
	Object->Position = Vector3(0, 20, 0);

	Instance->Run();
	
	return 0;
}