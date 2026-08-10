#pragma once
#include <Editor/UI/Panels/Assets/AssetEditor.h>
#include <Editor/UI/ShaderEditorProvider.h>
#include <kui/UI/UITextEditor.h>

namespace engine::editor
{
	class ShaderEditor : public AssetEditor
	{
	public:

		ShaderEditor(AssetRef ShaderFile);

		void Update();

		void Save() override;
		void OnThemeChanged() override;
		void OnResized() override;

	private:
		ShaderEditorProvider* Provider = nullptr;
		kui::UITextEditor* Editor = nullptr;
	};
}