#pragma once
#include <Editor/UI/Windows/IDialogWindow.h>

namespace engine::editor
{
	class PluginData
	{
	public:
		string Name;
		bool IsDev = false;
		bool IsExperimental = false;
	};

	class PluginManagerWindow : public IDialogWindow
	{
	public:
		PluginManagerWindow();
		// Inherited via IDialogWindow
		void Begin() override;
		void Update() override;
		void Destroy() override;

	private:
		std::vector<PluginData> Plugins;
	};
}