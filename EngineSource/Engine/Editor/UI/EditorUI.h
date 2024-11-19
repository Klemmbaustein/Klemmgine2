#ifdef EDITOR
#pragma once
#include <kui/UI/UIBackground.h>
#include "EditorPanel.h"
#include "StatusBar.kui.hpp"

namespace engine::editor
{
	struct EditorTheme
	{
		kui::Vec3f Text = 1;
		kui::Vec3f Background = kui::Vec3f(0.11f, 0.11f, 0.12f);
		kui::Vec3f DarkBackground = 0.05f;
		kui::Vec3f DarkBackground2 = 0.08f;
		kui::Vec3f LightBackground = 0.2f;
		kui::Vec3f BackgroundHighlight = kui::Vec3f(0.4f);
		kui::Vec3f Highlight1 = kui::Vec3f(0.5f, 0.5f, 1);
		kui::Vec3f HighlightDark = kui::Vec3f(0.15f, 0.15f, 0.35f);
		kui::Vec3f Highlight2 = 1;
		kui::Vec3f HighlightText = 0;

	};

	class EditorUI
	{
	public:

		static string DefaultFontName;

		kui::UIBox* MainBackground = nullptr;
		kui::UIBackground* MenuBar = nullptr;
		kui::UIBackground* StatusBar = nullptr;

		EditorPanel* RootPanel = nullptr;
		StatusBarElement* StatsBarElement;

		static EditorPanel* FocusedPanel;

		static EditorUI* Instance;
		static kui::Font* EditorFont;
		static kui::Font* MonospaceFont;
		static EditorTheme Theme;

		enum class StatusType
		{
			Info,
			Warning,
			Error
		};

		static void SetStatusMessage(string NewMessage, StatusType Type);

		EditorUI();
		static void UpdateTheme(kui::Window* Target);
		void Update();
		void UpdateBackgrounds();
	private:
		string CurrentStatus;
		static void SetStatusMainThread(string NewMessage, StatusType Type);
	};
}
#endif