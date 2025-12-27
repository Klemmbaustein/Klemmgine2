#pragma once
#include "IDialogWindow.h"
#include <Editor/UI/Settings/SettingsPage.h>

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

		void OnThemeChanged() override;

		void ShowPage(SettingsPage* Page);

		void GenerateTabs();

		std::vector<SettingsPage*> Pages;

		SettingsPage* ActivePage = nullptr;

		PropertyMenu* SettingsMenu = nullptr;

		kui::UIScrollBox* Sidebar = nullptr;
	};
}