#include "ScriptEditorSettingsPage.h"
#include <kui/UI/UIDropdown.h>
#include <Editor/Settings/EditorSettings.h>

using namespace kui;

engine::editor::ScriptEditorSettingsPage::ScriptEditorSettingsPage()
{
	this->Name = "Script Editor";

	HasMiniMap = Settings::GetInstance()->Script.GetSetting("miniMap", true).GetBool();
}

engine::editor::ScriptEditorSettingsPage::~ScriptEditorSettingsPage()
{
}

void engine::editor::ScriptEditorSettingsPage::Generate(PropertyMenu* Target, SettingsWindow* TargetWindow)
{
	Target->CreateNewHeading("Script Editor");

	std::vector Options = {
	UIDropdown::Option{
		.Name = "Same as interface",
	},
	UIDropdown::Option{
		.Name = "Dark",
	},
	UIDropdown::Option{
		.Name = "Light",
	},
	UIDropdown::Option{
		.Name = "Solarized",
	}
	};

	Target->AddBooleanEntry("Editor MiniMap", HasMiniMap, [this]() {
		Settings::GetInstance()->Script.SetSetting("miniMap", SerializedValue(HasMiniMap));
	});

	size_t Index = 0;

	Target->AddDropdownEntry("Theme", Options, [Target, TargetWindow](UIDropdown::Option o) {
	}, Index);
}
