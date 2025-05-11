#pragma once
#include "IDialogWindow.h"

namespace engine::editor
{
	class SettingsWindow : public IDialogWindow
	{
	public:
		SettingsWindow();
		virtual ~SettingsWindow();

		void Begin() override;
		void Update() override;
		void Destroy() override;

	};
}