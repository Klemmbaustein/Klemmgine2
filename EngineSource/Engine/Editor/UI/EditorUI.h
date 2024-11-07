#ifdef EDITOR
#pragma once
#include <kui/UI/UIBackground.h>
#include "EditorPanel.h"
#include "StatusBar.kui.hpp"

namespace engine::editor
{
	using namespace kui;
	struct EditorTheme
	{
		Vec3f Text = 1;
		Vec3f Background = 0.12f;
		Vec3f DarkBackground = 0.05f;
		Vec3f LightBackground = 0.2f;
		Vec3f Highlight1 = Vec3f(0.5f, 0.5f, 1);
		Vec3f HighlightDark = Vec3f(0.15f, 0.15f, 0.35f);
		Vec3f Highlight2 = 1;
		Vec3f HighlightText = 0;

	};

	class EditorUI
	{
	public:

		kui::UIBox* MainBackground = nullptr;
		kui::UIBackground* MenuBar = nullptr;
		kui::UIBackground* StatusBar = nullptr;

		EditorPanel* RootPanel = nullptr;
		StatusBarElement* StatsBarElement;

		static EditorUI* Instance;
		static kui::Font* EditorFont;
		static EditorTheme Theme;

		EditorUI();
		void Update();
		void UpdateBackgrounds();
	};
}
#endif