#include "EditorSettingsCategory.h"
#include <Editor/UI/Panels/Viewport.h>

engine::editor::EditorSettings::EditorSettings()
	: SettingsCategory("editor")
{
	ListenToSetting(this, "mouseSensitivity", [](SerializedValue v) {
		Viewport::MouseSensitivity = v.GetFloat();
	});
}
