#include "ConsoleSettingsPage.h"
#include <Core/Log.h>
#include <Editor/Settings/EditorSettings.h>

engine::editor::ConsoleSettingsPage::ConsoleSettingsPage()
{
	this->Name = "Console";
}

engine::editor::ConsoleSettingsPage::~ConsoleSettingsPage()
{
}

void engine::editor::ConsoleSettingsPage::Generate(PropertyMenu* Target, SettingsWindow* TargetWindow)
{
	VerboseLog = Log::IsVerbose;
	ClearLogWhenGameStarts = Settings::GetInstance()->Console.GetSetting("clearLogWhenGameStarts", false).GetBool();
	
	Target->CreateNewHeading("Console");
	Target->AddBooleanEntry("Verbose log", VerboseLog, [this]() {
		Settings::GetInstance()->Console.SetSetting("verboseLog", SerializedValue(VerboseLog));
	});
	Target->AddBooleanEntry("Clear log when game starts", ClearLogWhenGameStarts, [this]() {
		Settings::GetInstance()->Console.SetSetting("clearLogWhenGameStarts", SerializedValue(ClearLogWhenGameStarts));
	});
}
