#ifdef EDITOR
#pragma once
#include <kui/UI/UIBackground.h>
#include "EditorPanel.h"

namespace engine::editor
{
	class EditorUI
	{
	public:

		kui::UIBox* MainBackground = nullptr;
		kui::UIBackground* MenuBar = nullptr;
		kui::UIBackground* StatusBar = nullptr;

		EditorPanel* RootPanel = nullptr;

		static EditorUI* Instance;

		EditorUI();
		void Update();
	};
}
#endif