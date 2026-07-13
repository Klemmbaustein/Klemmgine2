#include "ScriptEditorSettingsPage.h"
#include <kui/UI/UIDropdown.h>
#include <Editor/Settings/EditorSettings.h>
#include <Editor/UI/Windows/SettingsWindow.h>
#include <Engine/Internal/PlatformGraphics.h>
#include <Core/Platform/Platform.h>

using namespace kui;

engine::editor::ScriptEditorSettingsPage::ScriptEditorSettingsPage()
{
	this->Name = "Script Editor";

	HasMiniMap = Settings::GetInstance()->Script.GetSetting("miniMap", true).GetBool();
	TrimWhitespace = Settings::GetInstance()->Script.GetSetting("trimWhitespace", true).GetBool();
	UseVerticalTabs = Settings::GetInstance()->Script.GetSetting("useVerticalTabs", true).GetBool();
	UseExternalEditor = Settings::GetInstance()->Script.GetSetting("useExternalEditor", false).GetBool();
	UseDefaultEditor = Settings::GetInstance()->Script.GetSetting("useDefaultEditor", true).GetBool();
	ExternalEditorCommand = Settings::GetInstance()->Script.GetSetting("externalEditorCommand", "").GetString();
	ExternalEditorArguments = Settings::GetInstance()->Script.GetSetting("externalEditorArguments", "").GetString();

#if WINDOWS
	VsDir = platform::GetCommandOutput("\"C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe\" -latest -property productPath");
	VsDir = str::Trim(str::RemoveChar(VsDir, '\n'));
#endif
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

	Target->AddDropdownEntry("Code appearance", Options, [Target, TargetWindow](UIDropdown::Option o) {
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
		Settings::GetInstance()->Script.SetSetting("trimWhitespace", TrimWhitespace);
	});

	Target->AddBooleanEntry("Use vertical tabs", UseVerticalTabs, [this]() {
		Settings::GetInstance()->Script.SetSetting("useVerticalTabs", UseVerticalTabs);
	});

	GenerateExternalEditor(Target, TargetWindow);
}

void engine::editor::ScriptEditorSettingsPage::GenerateExternalEditor(PropertyMenu* Target, SettingsWindow* TargetWindow)
{
	auto& Script = Settings::GetInstance()->Script;

	Target->CreateNewHeading("External Editor");

	Target->AddBooleanEntry("Use external editor program", UseExternalEditor, [this, TargetWindow] {
		Settings::GetInstance()->Script.SetSetting("useExternalEditor", UseExternalEditor);
		TargetWindow->ShowPage(this);
	});

	if (!UseExternalEditor)
	{
		return;
	}

	Target->AddBooleanEntry("Let OS select editor", UseDefaultEditor, [this, TargetWindow] {
		Settings::GetInstance()->Script.SetSetting("useDefaultEditor", UseDefaultEditor);
		TargetWindow->ShowPage(this);
	});

	if (!UseDefaultEditor)
	{
		std::vector Options = {
			UIDropdown::Option{
				.Name = "Custom",
			},
	#if WINDOWS
			UIDropdown::Option{
				.Name = "Visual Studio",
			},
	#endif
			UIDropdown::Option{
				.Name = "Visual Studio Code",
			},
		};

		Target->AddDropdownEntry("Presets", Options, [this, Target, TargetWindow](UIDropdown::Option o) {
#if WINDOWS
			if (o.Name == "Visual Studio")
			{
				this->ExternalEditorCommand = VsDir;
				this->ExternalEditorArguments = "/command \"File.OpenFolder {workspace}\" /command \"File.OpenFile {file}\" /Edit {file}";
				Settings::GetInstance()->Script.SetSetting("externalEditorCommand", ExternalEditorCommand);
				Settings::GetInstance()->Script.SetSetting("externalEditorArguments", ExternalEditorArguments);
				Target->UpdateProperties();
				Target->UpdateElement();
			}
#endif
			if (o.Name == "Visual Studio Code")
			{
				this->ExternalEditorArguments = "{workspace} --goto {file}";
				Settings::GetInstance()->Script.SetSetting("externalEditorArguments", ExternalEditorArguments);
				Target->UpdateProperties();
				Target->UpdateElement();
			}
		}, 0);
		Target->AddStringEntry("Editor executable", ExternalEditorCommand, [this] {
			Settings::GetInstance()->Script.SetSetting("externalEditorCommand", ExternalEditorCommand);
		});
		Target->AddButtonEntry("", "Browse", [this, Target] {
			auto result = platform::OpenFileDialog({ platform::FileDialogFilter{.Name = "Executable", .FileTypes = {"exe"}}});

			if (result.empty())
			{
				return;
			}

			ExternalEditorCommand = result[0];

			Settings::GetInstance()->Script.SetSetting("externalEditorCommand", ExternalEditorCommand);
			Target->UpdateProperties();
			Target->UpdateElement();
		});

		Target->AddStringEntry("Editor command line", ExternalEditorArguments, [this] {
			Settings::GetInstance()->Script.SetSetting("externalEditorArguments", ExternalEditorArguments);
		});
		Target->AddInfoEntry("", "Special values:\n{file}: file to open\n{workspace}: root assets directory");
	}
}
