#ifdef EDITOR
#pragma once
#include <kui/Vec3.h>
#include <kui/UISize.h>
#include <Core/Types.h>

namespace engine::editor
{
	struct EditorTheme
	{
		bool IsLight = false;
		kui::UISize CornerSize = 5_px;

		kui::Vec3f Text = 1;
		kui::Vec3f DarkText = 0.85f;
		kui::Vec3f Background = kui::Vec3f(0.11f, 0.11f, 0.12f);
		kui::Vec3f DarkBackground = 0.05f;
		kui::Vec3f DarkBackground2 = 0.08f;
		kui::Vec3f LightBackground = 0.2f;
		kui::Vec3f BackgroundHighlight = kui::Vec3f(0.4f);
		kui::Vec3f Highlight1 = kui::Vec3f(0.5f, 0.5f, 1);
		kui::Vec3f HighlightDark = kui::Vec3f(0.15f, 0.15f, 0.35f);
		kui::Vec3f Highlight2 = 1;
		kui::Vec3f HighlightText = 0;

		//kui::Vec3f Text = 0;
		//kui::Vec3f DarkText = 0.2f;
		//kui::Vec3f Background = kui::Vec3f(0.9f, 0.9f, 0.9f);
		//kui::Vec3f DarkBackground = 1.0f;
		//kui::Vec3f DarkBackground2 = 0.8f;
		//kui::Vec3f LightBackground = 0.8f;
		//kui::Vec3f BackgroundHighlight = kui::Vec3f(0.4f);
		//kui::Vec3f Highlight1 = kui::Vec3f(0.1f, 0.3f, 0.7f);
		//kui::Vec3f HighlightDark = kui::Vec3f(0.5f, 0.7f, 1.0f);
		//kui::Vec3f Highlight2 = 0.1f;
		//kui::Vec3f HighlightText = 1;

		void LoadFromFile(string ThemeName);
	};
}
#endif