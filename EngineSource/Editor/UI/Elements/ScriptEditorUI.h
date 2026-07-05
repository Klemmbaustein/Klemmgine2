#pragma once
#include <kui/UI/UITextEditor.h>
#include <Editor/UI/ScriptEditorProvider.h>
#include <Editor/UI/ScriptMiniMap.h>
#include <TextEditor.kui.hpp>
#include <Editor/UI/KeyboardShortcut.h>
#include <Core/ThreadMessages.h>
#include "Toolbar.h"

namespace engine::editor
{
	struct ScriptEditorTab
	{
	public:
		kui::UITextEditor* Editor = nullptr;
		ScriptEditorProvider* Provider = nullptr;
		ScriptMiniMap* MiniMap = nullptr;
		kui::UIText* TabName = nullptr;
		bool NeedsRefresh = false;
		bool IsSaved = true;
	};

	class ScriptEditorUI : public ScriptEditorContext, KeyboardShortcuts
	{
	public:

		ScriptEditorUI(kui::UIBox* Background, bool IsFloating, kui::Font* TextFont, kui::Font* ScriptFont,
			thread::ThreadMessagesRef Queue);
		~ScriptEditorUI();

		void Update();

		void AddTab(std::string File);

		void OpenTab(size_t Tab);

		// Inherited via ScriptEditorContext
		void NavigateTo(std::string File, std::optional<ds::TokenPos> At) override;
		void OnChange(const string& Name) override;
		void OnResized(kui::Vec2f NewSize);

		void UpdateColors();

		bool IsVisible = true;
		bool HasFocus = true;

		std::vector<kui::UIBox*> GetEditorBoxes();
		Toolbar* ScriptToolbar = nullptr;

	private:
		thread::ThreadMessagesRef Queue;
		kui::Font* TextFont = nullptr;
		kui::Font* ScriptFont = nullptr;

		kui::Vec2f Size;
		void UpdateTabSize(ScriptEditorTab* Tab);

		kui::UIBox* CenterBox = nullptr;
		kui::UIBox* EditorBox = nullptr;
		kui::UIBox* Background = nullptr;

		kui::UIScrollBox* VerticalTabBox = nullptr;
		kui::UIScrollBox* HorizontalTabBox = nullptr;
		kui::UIBackground* VerticalTabBackground = nullptr;
		TextEditorSearchElement* SearchUI = nullptr;
		TabSwitcher* Switcher = nullptr;
		SearchContext* CurrentSearch = nullptr;
		std::vector<TabSwitcherItem*> SwitchItems;

		bool SearchMatchCase = false;
		bool SearchGotAnyResults = false;

		kui::UIText* StatusText = nullptr;

		std::vector<ScriptEditorTab> Tabs;
		size_t SelectedTab = 0;

		ScriptEditorTab* GetSelectedTab();

		void CloseTab(size_t Index);

		void CloseSearch();
		void RunSearch();
		void UpdateSearchPosition();

		void OpenTabSwitcher();
		void CloseTabSwitcher();
		void SwitchToTab(size_t NewTabIndex);

		void InitializeSettings();

		string NameFormat;
		bool Saved = true;
		bool UseVerticalTabs = true;
		bool IsFloating = false;
		void UpdateEditorTabs();
		void Save();

		std::set<string> GetLastOpenedFiles();
		void SaveLastOpenedFiles();

		string GetOpenedTabsFile();

		kui::UIBackground* SeparatorBackgrounds[2];

		// Inherited via KeyboardShortcuts
		bool HasKeyboardFocus() override;
	};
}