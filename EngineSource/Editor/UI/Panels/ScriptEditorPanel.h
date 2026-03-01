#pragma once
#include "EditorPanel.h"
#include <kui/UI/UITextEditor.h>
#include <Editor/UI/ScriptEditorProvider.h>
#include <Editor/UI/ScriptMiniMap.h>

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

	class ScriptEditorPanel : public EditorPanel, public ScriptEditorContext
	{
	public:
		ScriptEditorPanel();
		~ScriptEditorPanel();

		virtual void OnResized() override;
		virtual void Update() override;

		void OnThemeChanged() override;

		void AddTab(std::string File);

		void OpenTab(size_t Tab);

		// Inherited via ScriptEditorContext
		void NavigateTo(std::string File, std::optional<ds::TokenPos> At) override;
		void OnChange(const string& Name) override;

	private:
		void UpdateTabSize(ScriptEditorTab* Tab);

		kui::UIBox* CenterBox = nullptr;
		kui::UIBox* EditorBox = nullptr;

		kui::UIScrollBox* TabBox = nullptr;

		kui::UIText* StatusText = nullptr;

		std::vector<ScriptEditorTab> Tabs;
		size_t SelectedTab = 0;

		ScriptEditorTab* GetSelectedTab();

		void CloseTab(size_t Index);

		string NameFormat;
		bool Saved = true;
		void UpdateEditorTabs();
		void Save();

		std::set<string> GetLastOpenedFiles();
		void SaveLastOpenedFiles();

		static constexpr auto TABS_OPENED_FILE = ".editor/tabs.json";

		kui::UIBackground* SeparatorBackgrounds[2];
	};
}