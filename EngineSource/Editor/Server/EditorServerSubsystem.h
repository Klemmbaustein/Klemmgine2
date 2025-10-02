#pragma once
#include <Engine/Subsystem/Subsystem.h>

namespace engine::editor
{
	class EditorServerSubsystem : public subsystem::Subsystem
	{
	public:

		EditorServerSubsystem();

		void Connect(string Url);
	};
}