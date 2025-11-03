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
	Engine* Instance = Engine::Init();

	Instance->GetSubsystem<SceneSubsystem>()->LoadSceneAsync("Assets/Test");

	Instance->Run();
	return 0;
}