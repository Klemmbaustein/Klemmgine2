#pragma once
#include <kui/UI/UITextEditor.h>
#include <Editor/UI/ScriptEditorProvider.h>

namespace engine::editor
{
	class ScriptMiniMap
	{
	public:

		ScriptMiniMap(kui::UITextEditor* Editor, ScriptEditorProvider* Provider);

		void Update();
		void GenerateTexture(uint32 ScrollBoxHeight);
		bool ReGenerate = false;

		kui::Vec3f BackgroundColor = 0.1f;

	private:

		void SetPixel(size_t x, size_t y, uByte R, uByte G, uByte B);

		std::vector<uByte> Texture;
		kui::UITextEditor* Editor = nullptr;
		ScriptEditorProvider* Provider = nullptr;
		size_t Width = 0;
		size_t Height = 0;
		size_t OldLength = 0;

		uint32 Image = 0;

		uByte BackgroundB = 0;
		uByte BackgroundR = 0;
		uByte BackgroundG = 0;

		size_t DownPadding = 0;

		bool IsGenerating = false;
	};
}