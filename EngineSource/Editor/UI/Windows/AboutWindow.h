#pragma once
#include "IDialogWindow.h"

namespace engine::editor
{
	class AboutWindow : public IDialogWindow
	{
	public:
		AboutWindow();

		// Inherited via IDialogWindow
		void Begin() override;
		void Update() override;
		void Destroy() override;
	};
}