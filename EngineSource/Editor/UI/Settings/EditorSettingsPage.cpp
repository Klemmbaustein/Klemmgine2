#include "EditorSettingsPage.h"
#include <Editor/UI/Windows/SettingsWindow.h>
#include <Core/Platform/Platform.h>
#include <filesystem>
#include <Editor/Editor.h>
#include <Core/File/FileUtil.h>
#include <Editor/Settings/EditorSettings.h>

engine::editor::EditorSettingsPage::EditorSettingsPage()
{
	Name = "Editor";
}

void engine::editor::EditorSettingsPage::Generate(PropertyMenu* Target, SettingsWindow* TargetWindow)
{
	string UseLocalSettingsFile = editor::GetEditorPath() + "/Config/useLocalSettings";

	UseGlobalConfigDir = !std::filesystem::exists(UseLocalSettingsFile);

	MouseSensitivity = Settings::GetInstance()->Editor.GetSetting("mouseSensitivity", 1.0f).GetFloat();

	Target->CreateNewHeading("Editor");

	Target->AddFloatEntry("Mouse sensitivity", MouseSensitivity, [this] {
		Settings::GetInstance()->Editor.SetSetting("mouseSensitivity", MouseSensitivity);
	});

	Target->AddBooleanEntry(str::Format("Use Global Settings Path", platform::GetConfigDir("").c_str()),
		UseGlobalConfigDir, [this, TargetWindow, UseLocalSettingsFile] {
		Settings::GetInstance()->Save();

		if (UseGlobalConfigDir)
		{
			std::filesystem::remove_all(UseLocalSettingsFile);
		}
		else
		{
			std::filesystem::create_directories(file::FilePath(UseLocalSettingsFile));
			std::ofstream s = std::ofstream(UseLocalSettingsFile);
			s.close();
		}

		Settings::GetInstance()->Reload();

		TargetWindow->ShowPage(TargetWindow->ActivePage);
	});

}
