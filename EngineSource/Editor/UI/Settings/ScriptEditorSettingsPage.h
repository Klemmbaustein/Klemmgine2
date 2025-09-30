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

	private:
	};
}