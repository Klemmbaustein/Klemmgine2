#include "Engine/Engine.h"
#include <Engine/Subsystem/SceneSubsystem.h>
#include <Editor/Server/EditorServerSubsystem.h>
#include <Editor/Server/ServerConnectDialog.h>
#include "Core/Types.h"
#include "Core/Networking/Http.h"
#include "Core/File/SerializedData.h"
#include "Core/File/JsonSerializer.h"

using namespace engine;
using engine::subsystem::SceneSubsystem;

int32 EngineMain(int argc, char** argv)
{
	auto res = editor::ServerConnectDialog::Show();

	Engine* Instance = Engine::Init();

	if (res.Connect)
	{
		auto ServerInterface = new editor::EditorServerSubsystem(res.Connection);
		Instance->LoadSubsystem(ServerInterface);
		// "wss://localhost:7274/ws"
	}

	//Instance->GetSubsystem<SceneSubsystem>()->LoadSceneAsync("Assets/Test");

	Instance->Run();
	return 0;
}