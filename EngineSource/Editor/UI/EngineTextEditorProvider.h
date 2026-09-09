#pragma once
#include <kui/UI/FileEditorProvider.h>
#include <kui/UI/TextEditor.h>
#include <Core/Types.h>
#include <stack>
#include <Editor/UI/DropdownMenu.h>
#include <ds/service/languageService.hpp>

namespace engine::editor
{
	class SearchContext;

	/**
	 * @brief
	 * A text editor provider that contains common engine functionality (right click menu, hover, auto complete)
	 */
	class EngineTextEditorProvider : public kui::FileEditorProvider
	{
	public:

		EngineTextEditorProvider(std::string File);
		virtual ~EngineTextEditorProvider() override;

		string EditedFile;

		void Update() override;
		void GetHighlightsForRange(size_t Begin, size_t Length) override;

		[[nodiscard]]
		virtual std::vector<DropdownMenu::Option> GetRightClickOptions(kui::EditorPosition At);

		enum class CompletionSource
		{
			TriggerChar,
			Shortcut,
			WordCompletion
		};

		virtual void OnRightClick();
		[[nodiscard]]
		virtual std::vector<ds::AutoCompleteResult> GetCompletionsAt(kui::EditorPosition At,
			CompletionSource Source);

		virtual void SetLine(size_t Index, const std::vector<kui::TextSegment>& NewLine) override;
		virtual void OnCursorMove(int64& Column, int64& Line, bool IsPages) override;

		void TrimWhitespace(size_t IgnoreLine);
		void ClearHovered();
		void CloseAutoComplete();

		SearchContext* StartSearch(string Text, bool MatchCase);

		virtual bool CanShowCompletions();

		bool IsChanged = false;
		bool AllowArrowKeys = true;

	protected:

		void UpdateAutoComplete();
		void UpdateAutoCompleteEntries(string Filter);
		void InsertCompletion(const ds::AutoCompleteResult& Result);

		void ShowAutoComplete(CompletionSource Source, string Filter = "");
		std::string ProcessInput(std::string Text) override;
		kui::UIBox* CreateHoverBox(kui::UIBox* Content, kui::EditorPosition At);

		void ApplyHoverBoxPosition(kui::UIBox* Target, kui::EditorPosition At);

		bool GetIsAutoCompleteActive() const
		{
			return IsAutoCompleteActive;
		}

		kui::UIBox* GetHoverBox() const
		{
			return HoverBox;
		}

		virtual size_t GetCompletionUsingLine()
		{
			return 0;
		}

	private:
		kui::UIBox* HoverBox = nullptr;
		kui::EditorPosition CompletePosition;

		size_t SelectedCompletionItem = 0;
		size_t OldSelectionLine = 0;

		kui::UIScrollBox* AutoCompleteBox = nullptr;
		std::vector<ds::AutoCompleteResult> Completions;
		std::vector<kui::UIButton*> CompletionButtons;
		bool IsAutoCompleteActive = false;
		bool IsWaitingForAutoComplete = false;
		string CompleteFilter;
		CompletionSource CompleteSource = CompletionSource::TriggerChar;
	};

	class SearchContext
	{
	public:

		SearchContext(EngineTextEditorProvider* Provider, string Query, bool MatchCase);

		std::optional<kui::EditorPosition> Next();

		kui::EditorPosition LastPosition;

		EngineTextEditorProvider* Provider;
		bool MatchCase = false;
		string Query;
	};
}