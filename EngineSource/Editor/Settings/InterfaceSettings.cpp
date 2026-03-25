#include "InterfaceSettings.h"
#include <Editor/UI/EditorUI.h>
#include <kui/Window.h>
#include <Engine/MainThread.h>

using namespace kui;

engine::editor::InterfaceSettings::InterfaceSettings()
	: SettingsCategory("interface")
{
	AddPrivateListener("theme", [](SerializedValue val) {
		EditorUI::Theme.LoadFromFile(val.GetString());
		if (thread::IsMainThread)
		{
			EditorUI::UpdateTheme(Window::GetActiveWindow(), true);
		}
		else
		{
			thread::ExecuteOnMainThread([] {
				EditorUI::UpdateTheme(Window::GetActiveWindow(), true);
			});
		}
	});
	ListenToSetting(this, "uiScale", [this](SerializedValue val) {
		Window::GetActiveWindow()->DPIMultiplier = val.GetFloat();
	});
}
