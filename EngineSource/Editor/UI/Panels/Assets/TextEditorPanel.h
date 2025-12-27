#pragma once
#include "AssetEditor.h"
#include <kui/UI/UITextEditor.h>
#include <Editor/UI/EngineTextEditorProvider.h>

namespace engine::editor
{
	class TextEditorPanel : public AssetEditor
	{
	public:
		TextEditorPanel(AssetRef Asset);
		~TextEditorPanel() override;

		virtual void OnResized() override;
		virtual void Save() override;

		void OnThemeChanged() override;

	private:

		kui::UITextEditor* Editor = nullptr;
		EngineTextEditorProvider* Provider = nullptr;
		string NameFormat;
		bool Saved = true;
	};
}