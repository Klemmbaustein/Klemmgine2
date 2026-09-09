#pragma once
#include <Engine/Script/ScriptSubsystem.h>
#include <Editor/UI/Elements/Toolbar.h>
#include <Editor/UI/KeyboardShortcut.h>

namespace engine::editor
{
	class DebuggerOverlay : KeyboardShortcuts
	{
	public:
		DebuggerOverlay(script::ScriptSubsystem* Scripts);

		~DebuggerOverlay();

		void Update();
		bool HasKeyboardFocus() override;

	private:
		Toolbar* DebuggerToolbar = nullptr;
		script::ScriptSubsystem* Scripts = nullptr;
	};
}