#include "ConsoleSettings.h"
#include <Core/Log.h>

engine::editor::ConsoleSettings::ConsoleSettings()
	: SettingsCategory("console")
{
	ListenToSetting(this, "verboseLog", [this](SerializedValue val) {
		Log::IsVerbose = val.GetBool();
	});
}
