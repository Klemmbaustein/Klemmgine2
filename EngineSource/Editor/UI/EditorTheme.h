#pragma once
#include <kui/Vec3.h>
#include <kui/UISize.h>
#include <Core/Types.h>
#include "CodeEditorTheme.h"

namespace engine::editor
{
	/**
	 * @brief Theme used by various editor elements.
	 *
	 * Loaded from a .k2t file, usually located in {editor path}/Editor/Themes
	 */
	struct EditorTheme
	{
		/// The name of the theme.
		string Name;
		/// The corner size is also controlled by the theme.
		kui::UISize CornerSize = 5_px;

		/// Regular text color.
		kui::Vec3f Text = 1;
		/// Text that is less highlighted than regular text.
		kui::Vec3f DarkText = 0.85f;
		/// Regular background color.
		kui::Vec3f Background = kui::Vec3f(0.11f, 0.11f, 0.12f);
		/// A background that is less highlighted than regular backgrounds.
		kui::Vec3f DarkBackground = 0.05f;
		/// A background that is way less highlighted than regular backgrounds.
		kui::Vec3f DarkBackground2 = 0.08f;
		/// A background that is more highlighted than regular backgrounds.
		kui::Vec3f LightBackground = 0.2f;
		/// A highlight color for the background, like the color of a separating line.
		kui::Vec3f BackgroundHighlight = kui::Vec3f(0.4f);
		/// A less intense background highlight color.
		kui::Vec3f DarkBackgroundHighlight = kui::Vec3f(0.3f);
		/// Highlights such as selected elements.
		kui::Vec3f Highlight1 = kui::Vec3f(0.5f, 0.5f, 1);
		/// A darker version of the highlight color.
		kui::Vec3f HighlightDark = kui::Vec3f(0.15f, 0.15f, 0.35f);
		/// Highlighted text.
		kui::Vec3f HighlightText = kui::Vec3f(0.8f, 0.7f, 1);

		/// If the theme is light, brighter colors will be used for the log and similar elements not covered by this theme.
		bool IsLight = false;

		/// The theme used for the code editor.
		CodeEditorTheme CodeTheme;

		/**
		 * @brief
		 * Loads a theme from a file
		 * @param ThemeName
		 * The path to the theme relative to Editor/Themes
		 */
		void LoadFromFile(string ThemeName);
	};
}
