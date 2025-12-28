#include "ScriptSettings.h"
#include "EditorSettings.h"

engine::editor::ScriptSettings::ScriptSettings()
	: SettingsCategory("script")
{
	ListenToSetting(this, "useMainThemeColors", [this](SerializedValue val) {
		Settings::GetInstance()->Interface.UpdateSetting("theme");
	});
}
