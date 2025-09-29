#include "InterfaceSettingsPage.h"

engine::editor::InterfaceSettingsPage::InterfaceSettingsPage()
{
	this->Name = "Interface";
}

engine::editor::InterfaceSettingsPage::~InterfaceSettingsPage()
{
}

void engine::editor::InterfaceSettingsPage::Generate(PropertyMenu* Target)
{
	Target->CreateNewHeading("Interface");
	Target->AddIntEntry("UI Scale %", UIScale, nullptr);
	Target->AddBooleanEntry("Theme", IsDark, nullptr);
}
