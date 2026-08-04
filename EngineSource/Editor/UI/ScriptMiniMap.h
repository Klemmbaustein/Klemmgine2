#pragma once
#include <kui/UI/UITextEditor.h>
#include <Editor/UI/ScriptEditorProvider.h>
#include <Core/ThreadMessages.h>
#include <memory>

namespace engine::editor
{
	class ScriptMiniMap
	{
	public:

		ScriptMiniMap(kui::UITextEditor* Editor, ScriptEditorProvider* Provider,
			thread::ThreadMessagesRef Queue);

		~ScriptMiniMap();

		void Update();
		void GenerateTexture(uint32 ScrollBoxHeight);
		bool ReGenerate = false;

		kui::Vec3f BackgroundColor = 0.1f;

	private:

		struct MiniMapBuildData
		{
			std::mutex m;
			bool IsLoaded = false;
		};

		void SetPixel(size_t x, size_t y, uByte R, uByte G, uByte B);
		thread::ThreadMessagesRef Queue;
		std::vector<uByte> Texture;
		kui::UITextEditor* Editor = nullptr;
		ScriptEditorProvider* Provider = nullptr;
		std::shared_ptr<MiniMapBuildData> Data = std::make_shared<MiniMapBuildData>();
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