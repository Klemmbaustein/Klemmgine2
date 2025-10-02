#include "Engine/Engine.h"
#include <Engine/Subsystem/SceneSubsystem.h>
#include <Editor/Server/EditorServerSubsystem.h>
#include "Core/Types.h"
#include "Core/Networking/Http.h"

using namespace engine;
using engine::subsystem::SceneSubsystem;

int32 EngineMain(int argc, char** argv)
{
	Engine* Instance = Engine::Init();
	auto ServerInterface = new editor::EditorServerSubsystem();
	Instance->LoadSubsystem(ServerInterface);

	ServerInterface->Connect("http://localhost:5149/ws");

	//Instance->GetSubsystem<SceneSubsystem>()->LoadSceneAsync("Assets/Test");

	Instance->Run();
	return 0;
}