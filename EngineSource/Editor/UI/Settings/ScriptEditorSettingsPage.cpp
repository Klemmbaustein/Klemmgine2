#include "ScriptEditorSettingsPage.h"
#include <kui/UI/UIDropdown.h>
#include <Editor/Settings/EditorSettings.h>

using namespace kui;

engine::editor::ScriptEditorSettingsPage::ScriptEditorSettingsPage()
{
	this->Name = "Script Editor";

	HasMiniMap = Settings::GetInstance()->Script.GetSetting("miniMap", true).GetBool();
	TrimWhitespace = Settings::GetInstance()->Script.GetSetting("trimWhitespace", true).GetBool();
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
		.Name = "Klemmgine Dark",
	},
	UIDropdown::Option{
		.Name = "Klemmgine Light",
	},
	UIDropdown::Option{
		.Name = "Solarized",
	},
	UIDropdown::Option{
		.Name = "Catppuccin",
	},
	UIDropdown::Option{
		.Name = "Visual Studio Dark",
	},
	};

	auto& Script = Settings::GetInstance()->Script;

	Target->AddBooleanEntry("Editor MiniMap", HasMiniMap, [this]() {
		Settings::GetInstance()->Script.SetSetting("miniMap", SerializedValue(HasMiniMap));
	});

	auto Index = std::ranges::find_if(Options,
		[](const UIDropdown::Option& o) {
		return o.Name == EditorUI::Theme.CodeTheme.Name;
	});

	Target->AddDropdownEntry("Theme", Options, [Target, TargetWindow](UIDropdown::Option o) {
		auto& Script = Settings::GetInstance()->Script;

		if (o.Name == "Same as interface")
		{
			Script.SetSetting("useMainThemeColors", true);
		}
		else
		{
			Script.SetSetting("useMainThemeColors", false);
			Script.SetSetting("editorTheme", o.Name);
		}
	}, Script.GetSetting("useMainThemeColors", true).GetBool() ? 0 : std::distance(Options.begin(), Index));

	Target->AddBooleanEntry("Trim whitespace on save", TrimWhitespace, [this]() {
		Settings::GetInstance()->Script.SetSetting("trimWhitespace", SerializedValue(TrimWhitespace));
	});
}
