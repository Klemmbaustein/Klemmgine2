#pragma once
#include "DialogWindow.h"

namespace engine::editor
{
	class AboutWindow : public DialogWindow
	{
	public:
		AboutWindow();

		// Inherited via DialogWindow
		void Begin() override;
		void Update() override;
		void Destroy() override;
	};
}