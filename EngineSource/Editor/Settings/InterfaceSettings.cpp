#include "InterfaceSettings.h"
#include <Editor/UI/EditorUI.h>
#include <kui/Window.h>

using namespace kui;

engine::editor::InterfaceSettings::InterfaceSettings()
	: SettingsCategory("interface")
{
	ListenToSetting(this, "theme", [this](SerializedValue val) {
		EditorUI::Theme.LoadFromFile(val.GetString());
		EditorUI::UpdateTheme(Window::GetActiveWindow(), true);
	});
	ListenToSetting(this, "uiScale", [this](SerializedValue val) {
		Window::GetActiveWindow()->DPIMultiplier = val.GetFloat();
	});
}
