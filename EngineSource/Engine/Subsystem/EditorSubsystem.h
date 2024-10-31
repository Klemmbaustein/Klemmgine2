#ifdef EDITOR
#pragma once
#include "ISubsystem.h"
#include <Engine/Editor/UI/EditorUI.h>

namespace engine::subsystem
{
	class EditorSubsystem : public ISubsystem
	{
	public:
		EditorSubsystem();

		virtual void Update() override;
		
		editor::EditorUI* UI;
	};
}
#endif