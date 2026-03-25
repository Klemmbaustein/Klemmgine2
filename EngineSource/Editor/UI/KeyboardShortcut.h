#pragma once
#include <kui/Input.h>
#include <optional>
#include <functional>

namespace engine::editor
{
	class KeyboardShortcuts
	{
	public:
		KeyboardShortcuts();
		~KeyboardShortcuts();

		enum class ShortcutOptions
		{
			/// No options.
			None = 0,
			/// Allow the shortcut to activate while focus is in a text box.
			AllowInText = 0b01,
			/// The shortcut is global and doesn't require the panel to be focused.
			Global = 0b10,
		};

		class ShortcutModifiers
		{
		public:
			bool Shift : 1 = false;
			bool Ctrl : 1 = false;
			bool Alt : 1 = false;
		};

		/**
		 * @brief
		 * Creates a keyboard shortcut for this panel.
		 *
		 * The shortcut
		 * @param NewKey
		 * The key that the shortcut listens to. When it is pressed, the shortcut activates.
		 * @param Modifiers
		 * Modifiers for the shortcut that also needs to be pressed when the shortcut is activated.
		 * Usually Shift or Ctrl.
		 * @param OnPressed
		 * The function to call once the shortcut was triggered.
		 * @param Options
		 * Options for the shortcut.
		 */
		void AddShortcut(kui::Key NewKey, ShortcutModifiers Modifiers, std::function<void()> OnPressed,
			ShortcutOptions Options = ShortcutOptions::None);

		virtual bool HasKeyboardFocus() = 0;

	private:

		std::vector<kui::Key> Shortcuts;
	};
}

inline engine::editor::KeyboardShortcuts::ShortcutOptions operator|(engine::editor::KeyboardShortcuts::ShortcutOptions a,
	engine::editor::KeyboardShortcuts::ShortcutOptions b)
{
	return engine::editor::KeyboardShortcuts::ShortcutOptions(int(a) | int(b));
}
