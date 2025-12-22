#pragma once
#include <Core/Event.h>
#include <ds/language.hpp>
#include <Editor/Server/ServerConnection.h>
#include <kui/Timer.h>
#include <kui/UI/FileEditorProvider.h>
#include <kui/UI/UIScrollBox.h>
#include <stack>
#include "ScriptEditorContext.h"

namespace engine::editor
{
	struct ScriptSyntaxHighlight
	{
		size_t Start = 0;
		size_t Length = 0;
		kui::Vec3f Color;
	};

	class ScriptEditorProvider : public kui::FileEditorProvider
	{
	public:
		ScriptEditorProvider(string ScriptFile, ScriptEditorContext* Context);
		ScriptEditorProvider(const ScriptEditorProvider&) = delete;
		~ScriptEditorProvider();

		string ScriptFile;
		ScriptEditorContext* Context = nullptr;

		void GetHighlightsForRange(size_t Begin, size_t Length) override;
		void RemoveLines(size_t Start, size_t Length) override;
		void SetLine(size_t Index, const std::vector<kui::TextSegment>& NewLine) override;
		void InsertLine(size_t Index, const std::vector<kui::TextSegment>& Content) override;

		void GetLine(size_t LineIndex, std::vector<kui::TextSegment>& To) override;
		void OnLoaded() override;
		void Update() override;

		kui::Vec3f TypeColor = kui::Vec3f(1.0f, 0.8f, 0.1f);
		kui::Vec3f VariableColor = kui::Vec3f(0.71f, 0.96f, 0.95f);
		kui::Vec3f FunctionColor = kui::Vec3f(0.27f, 0.88f, 0.65f);

		kui::UIBox* CreateHoverBox(kui::UIBox* Content, kui::EditorPosition At);
		ServerConnection* Connection = nullptr;

		void UpdateAutoComplete();

		void ShowAutoComplete(bool MembersOnly, std::string Filter = "");

		void LoadRemoteFile();

		void Commit() override;

		void RefreshAll();
		void ScanFile();

		void Undo();
		void Redo();

		void ShowDefinitionAt(kui::EditorPosition Position);

		std::string ProcessInput(std::string Text) override;

		struct HoverErrorData
		{
			ScriptError* Error = nullptr;
			kui::EditorPosition At;
		};

		struct HoverSymbolData
		{
			void* Symbol = nullptr;
			kui::EditorPosition At;
			std::function<kui::UIBox* ()> GetHoverData;
			std::function<ds::Token()> GetDefinition;
		};

		Event<> OnUpdated;
		HoverSymbolData GetSymbolAt(kui::EditorPosition Position);

		HoverErrorData GetHoveredError(kui::Vec2f ScreenPosition);
		HoverSymbolData GetHoveredSymbol(kui::Vec2f ScreenPosition);

		void NavigateTo(kui::EditorPosition Position);
		void NavigateTo(kui::EditorPosition StartPosition, kui::EditorPosition EndPosition);

		void CloseAutoComplete();

	private:

		void InsertCompletion(string CompletionText);

		struct ChangePart
		{
			uint64_t Line;
			string Content;
			bool IsRemove = false;
			bool IsAdd = false;
		};

		struct Change
		{
			std::vector<ChangePart> Parts;
		};

		Change ApplyChange(const Change& Target);

		Change NextChange;

		std::stack<Change> Changes;
		std::stack<Change> UnDoneChanges;

		std::vector<size_t> Changed;

		kui::UIScrollBox* AutoCompleteBox = nullptr;

		kui::Timer HoverTime;

		bool IsAutoCompleteActive = false;

		void* HoveredData = nullptr;

		kui::UIBox* HoveredBox = nullptr;
		kui::Vec2f LastCursorPosition;

		std::vector<ds::AutoCompleteResult> Completions;
		std::vector<kui::UIButton*> CompletionButtons;
		std::map<size_t, std::vector<ScriptSyntaxHighlight>> Highlights;
		void UpdateLineColorization(size_t Line);

		void ApplyHoverBoxPosition(kui::UIBox* Target, kui::EditorPosition At);
		void UpdateAutoCompleteEntries(string Filter);
		kui::EditorPosition CompletePosition;

		void UpdateFile();
		void UpdateSyntaxHighlight();
	};
}