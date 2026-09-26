#pragma once
#include "SettingsPage.h"

namespace engine::editor
{
	class EditorSettingsPage : public SettingsPage
	{
	public:

		EditorSettingsPage();

		// Inherited via SettingsPage
		void Generate(PropertyMenu* Target, SettingsWindow* TargetWindow) override;

		bool UseGlobalConfigDir = false;
		float MouseSensitivity = 1.0f;
	};
}