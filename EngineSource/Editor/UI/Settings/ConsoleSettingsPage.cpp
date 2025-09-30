#include "ConsoleSettingsPage.h"
#include <Core/Log.h>
#include <Engine/MainThread.h>
#include <iostream>
#if WINDOWS
#include <Windows.h>
#endif

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
	Target->CreateNewHeading("Console");
	Target->AddBooleanEntry("Verbose log", VerboseLog, [this]() {
		Log::IsVerbose = VerboseLog;
	});
}
