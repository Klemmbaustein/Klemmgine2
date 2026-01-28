#pragma once
#include <kui/UI/FileEditorProvider.h>
#include <kui/UI/TextEditor.h>
#include <Core/Types.h>
#include <stack>
#include <Editor/UI/DropdownMenu.h>
#include <ds/service/languageService.hpp>

namespace engine::editor
{
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

		virtual void Update();

		virtual std::vector<DropdownMenu::Option> GetRightClickOptions(kui::EditorPosition At);

		enum class CompletionSource
		{
			TriggerChar,
			Shortcut,
			WordCompletion
		};

		virtual void OnRightClick();
		virtual std::vector<ds::AutoCompleteResult> GetCompletionsAt(kui::EditorPosition At,
			CompletionSource Source);

		virtual void SetLine(size_t Index, const std::vector<kui::TextSegment>& NewLine) override;

		void TrimWhitespace(size_t IgnoreLine);
		void ClearHovered();

	protected:

		void UpdateAutoComplete();
		void CloseAutoComplete();
		void UpdateAutoCompleteEntries(string Filter);
		void InsertCompletion(string CompletionText);

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

	private:
		kui::UIBox* HoverBox = nullptr;
		kui::EditorPosition CompletePosition;

		kui::UIScrollBox* AutoCompleteBox = nullptr;
		std::vector<ds::AutoCompleteResult> Completions;
		std::vector<kui::UIButton*> CompletionButtons;
		bool IsAutoCompleteActive = false;
	};
}