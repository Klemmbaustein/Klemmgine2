#pragma once
#include "EditorPanel.h"
#include <Editor/UI/Elements/ScriptEditorUI.h>

namespace engine::editor
{

	class ScriptEditorPanel : public EditorPanel
	{
	public:
		ScriptEditorPanel();
		~ScriptEditorPanel();

		virtual void OnResized() override;
		virtual void Update() override;

		void OnThemeChanged() override;

		ScriptEditorUI UI;
	};
}