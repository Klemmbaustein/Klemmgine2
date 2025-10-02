#include "EditorServerSubsystem.h"
#include <Core/Networking/HttpWebSocket.h>
using namespace engine::editor;

EditorServerSubsystem::EditorServerSubsystem()
	: subsystem::Subsystem("Editor Server", Log::LogColor::Yellow)
{
}

void EditorServerSubsystem::Connect(string Url)
{
	auto c = http::WebSocketConnection(Url);
}
