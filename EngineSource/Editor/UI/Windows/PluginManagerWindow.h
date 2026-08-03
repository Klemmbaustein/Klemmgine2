#pragma once
#include <Editor/UI/Windows/IDialogWindow.h>

namespace engine::editor
{
	class PluginManagerWindow : public IDialogWindow
	{
	public:
		PluginManagerWindow();
		// Inherited via IDialogWindow
		void Begin() override;
		void Update() override;
		void Destroy() override;
	};
}