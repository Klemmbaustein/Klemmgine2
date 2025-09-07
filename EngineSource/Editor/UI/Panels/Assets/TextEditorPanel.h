#pragma once
#include "AssetEditor.h"
#include <kui/UI/UITextEditor.h>
#include <kui/UI/FileEditorProvider.h>

namespace engine::editor
{
	class TextEditorPanel : public AssetEditor
	{
	public:
		TextEditorPanel(AssetRef Asset);

		virtual void OnResized() override;
		virtual void Save() override;

	private:

		kui::UITextEditor* Editor = nullptr;
		kui::FileEditorProvider* Provider = nullptr;
		string NameFormat;
		bool Saved = true;
		void UpdateName();
		void RunScript();
	};
}