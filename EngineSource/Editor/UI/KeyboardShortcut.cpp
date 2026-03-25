#include "KeyboardShortcut.h"
#include <kui/Window.h>
#include <Engine/Input.h>

using namespace kui;

engine::editor::KeyboardShortcuts::KeyboardShortcuts()
{
}

engine::editor::KeyboardShortcuts::~KeyboardShortcuts()
{
	for (auto& s : Shortcuts)
	{
		Window::GetActiveWindow()->Input.RemoveOnKeyDownCallback(s, this);
	}
}

void engine::editor::KeyboardShortcuts::AddShortcut(kui::Key NewKey, ShortcutModifiers Modifiers,
	std::function<void()> OnPressed, ShortcutOptions Options)
{
	bool AllowInText = (int(Options) & int(ShortcutOptions::AllowInText)) > 0;
	bool Global = (int(Options) & int(ShortcutOptions::Global)) > 0;

	Window::GetActiveWindow()->Input.RegisterOnKeyDownCallback(NewKey, this,
		[this, OnPressed, Modifiers, AllowInText, Global]() {

		auto TargetWindow = Window::GetActiveWindow();

		if (!input::ShowMouseCursor && !AllowInText)
		{
			return;
		}

		if (!AllowInText && TargetWindow->Input.PollForText)
		{
			return;
		}

		if (!Global && !HasKeyboardFocus())
		{
			return;
		}

		if (Modifiers.Ctrl && !TargetWindow->Input.IsKeyDown(Key::CTRL))
		{
			return;
		}
		if (Modifiers.Shift && !TargetWindow->Input.IsKeyDown(Key::SHIFT))
		{
			return;
		}
		if (Modifiers.Alt && !TargetWindow->Input.IsKeyDown(Key::ALT))
		{
			return;
		}
		OnPressed();
	});
	Shortcuts.push_back(NewKey);
}
