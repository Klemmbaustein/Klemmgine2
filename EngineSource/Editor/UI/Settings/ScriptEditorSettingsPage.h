#pragma once
#include "SettingsPage.h"

namespace engine::editor
{
	class ScriptEditorSettingsPage : public SettingsPage
	{
	public:
		ScriptEditorSettingsPage();
		~ScriptEditorSettingsPage() override;

		void Generate(PropertyMenu* Target, SettingsWindow* TargetWindow) override;
		void GenerateExternalEditor(PropertyMenu* Target, SettingsWindow* TargetWindow);

	private:

		bool HasMiniMap = true;
		bool TrimWhitespace = true;
		bool UseVerticalTabs = true;
		bool UseExternalEditor = false;
		bool UseDefaultEditor = false;
		string ExternalEditorCommand;
		string ExternalEditorArguments;

		string VsDir = "";
	};
}