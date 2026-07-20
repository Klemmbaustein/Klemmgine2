#pragma once
#include "SettingsPage.h"
#include <mutex>

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
		static bool CheckedVsDir;
		string ExternalEditorCommand;
		string ExternalEditorArguments;

		string VsDir = "";
		std::mutex VsCheckMutex;
	};
}