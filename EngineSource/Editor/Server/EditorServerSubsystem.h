#pragma once
#include <Engine/Subsystem/Subsystem.h>
#include "ServerConnection.h"

namespace engine::editor
{
	class EditorUI;

	class EditorServerSubsystem : public subsystem::Subsystem
	{
	public:

		EditorServerSubsystem(ServerConnection* Connection);
		~EditorServerSubsystem() override;

		void OnEditorLoaded(EditorUI* UI);
		ServerConnection* Connection = nullptr;
	};
}