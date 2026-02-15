#pragma once
#include "SettingsPage.h"

namespace engine::editor
{
	class InterfaceSettingsPage : public SettingsPage
	{
	public:
		InterfaceSettingsPage();
		~InterfaceSettingsPage() override;

		void Generate(PropertyMenu* Target, SettingsWindow* TargetWindow) override;

	private:
		int32 UIScale = 100;
		bool ShowDevWarning = true;
	};
}