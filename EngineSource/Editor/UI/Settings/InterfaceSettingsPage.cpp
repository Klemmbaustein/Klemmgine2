#include "InterfaceSettingsPage.h"
#include <Editor/Settings/EditorSettings.h>
#include <Editor/UI/Windows/SettingsWindow.h>
#include <Editor/UI/EditorUI.h>
#include <Engine/MainThread.h>

using namespace kui;

engine::editor::InterfaceSettingsPage::InterfaceSettingsPage()
{
	this->Name = "Interface";

	UIScale = Settings::GetInstance()->Interface.GetSetting("uiScale", 1.0f).GetFloat() * 100.0f;
	ShowDevWarning = Settings::GetInstance()->Interface.GetSetting("showDevWarning", true).GetBool();
}

engine::editor::InterfaceSettingsPage::~InterfaceSettingsPage()
{
}

void engine::editor::InterfaceSettingsPage::Generate(PropertyMenu* Target, SettingsWindow* TargetWindow)
{
	Target->CreateNewHeading("Interface");
	Target->AddIntEntry("UI Scale %", UIScale, [this]() {
		Settings::GetInstance()->Interface.SetSetting("uiScale", float(UIScale / 100.0f));
	});

	std::vector Options = {
	UIDropdown::Option{
		.Name = "Dark",
	},
	UIDropdown::Option{
		.Name = "Light",
	},
	UIDropdown::Option{
		.Name = "Solarized",
	},
	UIDropdown::Option{
		.Name = "Windows 10",
	},
	UIDropdown::Option{
		.Name = "Windows 10 Dark",
	},
	UIDropdown::Option{
		.Name = "Catppuccin",
	},
	};

	auto Index = std::ranges::find_if(Options, [](const UIDropdown::Option& o) {
		return o.Name == EditorUI::Theme.Name;
	});

	Target->AddDropdownEntry("Theme", Options, [Target, TargetWindow](UIDropdown::Option o) {
		Settings::GetInstance()->Interface.SetSetting("theme", o.Name);
	}, std::distance(Options.begin(), Index));
	Target->AddBooleanEntry("Show \"In development\" warning on startup", ShowDevWarning, [this]() {
		Settings::GetInstance()->Interface.SetSetting("showDevWarning", ShowDevWarning);
	});
}
