#pragma once
#include "EditorPanel.h"
#include <kui/UI/UITextEditor.h>
#include <Editor/UI/ScriptEditorProvider.h>
#include <Editor/UI/ScriptMiniMap.h>

namespace engine::editor
{
	class ScriptEditorPanel : public EditorPanel
	{
	public:
		ScriptEditorPanel();

		virtual void OnResized() override;
		virtual void Update() override;

		void OnThemeChanged() override;

	private:

		kui::UIBox* CenterBox = nullptr;

		kui::UITextEditor* Editor = nullptr;
		kui::UIScrollBox* TabBox = nullptr;
		ScriptEditorProvider* Provider = nullptr;

		ScriptMiniMap* MiniMap = nullptr;

		kui::UIText* StatusText = nullptr;

		string NameFormat;
		bool Saved = true;
		void UpdateTabs();
		void Save();

		kui::UIBackground* SeparatorBackgrounds[2];
	};
}