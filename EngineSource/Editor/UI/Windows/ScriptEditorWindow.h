#pragma once
#include "IPopupWindow.h"
#include <Editor/UI/Elements/ScriptEditorUI.h>

namespace engine::editor
{
	class ScriptEditorWindow : public IPopupWindow
	{
	public:
		ScriptEditorWindow();

		void Begin() override;
		void Update() override;
		void Destroy() override;

		void OnThemeChanged() override;

		ScriptEditorUI* UI = nullptr;

		static ScriptEditorWindow* Current;

	private:

		kui::UIBackground* Background = nullptr;
		kui::Font* MonospacedFont = nullptr;
	};
}
