#include "InterfaceSettingsPage.h"
#include <Editor/UI/Windows/SettingsWindow.h>
#include <Editor/UI/EditorUI.h>
#include <Engine/MainThread.h>

using namespace kui;

engine::editor::InterfaceSettingsPage::InterfaceSettingsPage()
{
	this->Name = "Interface";
}

engine::editor::InterfaceSettingsPage::~InterfaceSettingsPage()
{
}

void engine::editor::InterfaceSettingsPage::Generate(PropertyMenu* Target, SettingsWindow* TargetWindow)
{
	Target->CreateNewHeading("Interface");
	Target->AddIntEntry("UI Scale %", UIScale, nullptr);

	std::vector Options = {
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

	auto Index = std::ranges::find_if(Options,
		[](const UIDropdown::Option& o) {
		return o.Name == EditorUI::Theme.Name;
	});

	Target->AddDropdownEntry("Theme", Options, [Target, TargetWindow](UIDropdown::Option o) {
		EditorUI::Instance->Theme.LoadFromFile(o.Name);
		EditorUI::Instance->UpdateTheme(Target->GetParentWindow(), true);
		TargetWindow->ShowPage(TargetWindow->ActivePage);

		thread::ExecuteOnMainThread([] {
			EditorUI::Instance->UpdateTheme(Window::GetActiveWindow(), true);
		});
	}, std::distance(Options.begin(), Index));
}
