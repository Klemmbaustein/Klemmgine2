#ifdef EDITOR
#pragma once
#include "EditorPanel.h"

class ConsolePanelElement;

namespace engine::editor
{
	class ConsolePanel : public EditorPanel
	{
	public:

		ConsolePanel();
		~ConsolePanel();

		ConsolePanelElement* Element = nullptr;

		virtual void Update() override;
		virtual void OnResized() override;
		virtual void OnThemeChanged() override;
	private:

		std::string Filter;

		size_t LastLogSize = 0;
		void UpdateLog(bool Full);
	};
}
#endif