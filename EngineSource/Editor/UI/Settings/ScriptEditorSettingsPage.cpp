#include "ScriptEditorSettingsPage.h"
#include <kui/UI/UIDropdown.h>

using namespace kui;

engine::editor::ScriptEditorSettingsPage::ScriptEditorSettingsPage()
{
	this->Name = "Script Editor";
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

	Target->AddBooleanEntry("Editor MiniMap", HasMiniMap, nullptr);

	size_t Index = 0;

	Target->AddDropdownEntry("Theme", Options, [Target, TargetWindow](UIDropdown::Option o) {
	}, Index);
}
