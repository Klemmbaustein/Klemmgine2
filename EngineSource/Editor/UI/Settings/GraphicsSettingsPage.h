#pragma once
#include "SettingsPage.h"

namespace engine::editor
{
	class GraphicsSettingsPage : public SettingsPage
	{
	public:
		GraphicsSettingsPage();
		~GraphicsSettingsPage() override;

		void Generate(PropertyMenu* Target) override;

	private:
		bool AntiAliasing = false;
	};
}