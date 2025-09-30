#pragma once
#include "SettingsPage.h"

namespace engine::editor
{
	class ConsoleSettingsPage : public SettingsPage
	{
	public:
		ConsoleSettingsPage();
		~ConsoleSettingsPage() override;

		void Generate(PropertyMenu* Target, SettingsWindow* TargetWindow) override;

	private:
		bool VerboseLog = false;
	};
}