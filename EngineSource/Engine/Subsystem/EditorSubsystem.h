#ifdef EDITOR
#pragma once
#include "Subsystem.h"
#include <Editor/UI/EditorUI.h>

namespace engine::subsystem
{
	class EditorSubsystem : public Subsystem
	{
	public:
		EditorSubsystem();
		virtual ~EditorSubsystem() override;

		virtual void Update() override;
		
		void StartProject();
		void StopProject();

		editor::EditorUI* UI = nullptr;

		static bool Active;
	};
}
#endif