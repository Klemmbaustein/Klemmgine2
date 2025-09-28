#pragma once
#include <Engine/Subsystem/Subsystem.h>
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
