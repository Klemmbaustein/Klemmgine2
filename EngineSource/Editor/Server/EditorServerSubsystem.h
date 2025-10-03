#pragma once
#include <Engine/Subsystem/Subsystem.h>
#include <Core/Networking/HttpWebSocket.h>
#include <Core/File/SerializedData.h>
#include "ServerConnection.h"

namespace engine::editor
{
	class EditorServerSubsystem : public subsystem::Subsystem
	{
	public:

		EditorServerSubsystem(ServerConnection* Connection);
		~EditorServerSubsystem() override;

		ServerConnection* Connection = nullptr;
	};
}