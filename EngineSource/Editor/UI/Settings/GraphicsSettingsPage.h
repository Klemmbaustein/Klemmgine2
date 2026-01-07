#pragma once
#include "SettingsPage.h"

namespace engine::editor
{
	class GraphicsSettingsPage : public SettingsPage
	{
	public:
		GraphicsSettingsPage();
		~GraphicsSettingsPage() override;

		void Generate(PropertyMenu* Target, SettingsWindow* TargetWindow) override;

	private:
		bool Vsync = true;
		bool Shadows = true;
		bool AmbientOcclusion = true;
	};
}