#pragma once
#include <Core/Event.h>
#include <Editor/Server/ServerConnection.h>
#include <kui/Timer.h>
#include "EngineTextEditorProvider.h"
#include "ScriptEditorContext.h"

namespace engine::editor
{
	struct ScriptSyntaxHighlight
	{
		size_t Start = 0;
		size_t Length = 0;
		kui::Vec3f Color;
	};

	struct SymbolDefinition
	{
		ds::Token Token;
		string File;
	};

	class ScriptEditorProvider : public EngineTextEditorProvider
	{
	public:
		ScriptEditorProvider(string ScriptFile, ScriptEditorContext* Context);
		ScriptEditorProvider(const ScriptEditorProvider&) = delete;
		~ScriptEditorProvider();

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

		ServerConnection* Connection = nullptr;
		ScriptEditorContext* Context = nullptr;

		void Commit() override;

		void RefreshAll();
		void ScanFile();

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
			std::function<SymbolDefinition()> GetDefinition;
		};

		Event<> OnUpdated;
		HoverSymbolData GetSymbolAt(kui::EditorPosition Position);

		HoverErrorData GetHoveredError(kui::Vec2f ScreenPosition);
		HoverSymbolData GetHoveredSymbol(kui::Vec2f ScreenPosition);

		void NavigateTo(kui::EditorPosition Position);
		void NavigateTo(kui::EditorPosition StartPosition, kui::EditorPosition EndPosition);

		std::vector<DropdownMenu::Option> GetRightClickOptions(kui::EditorPosition At) override;
		void OnRightClick() override;

		std::vector<ds::AutoCompleteResult> GetCompletionsAt(kui::EditorPosition At,
			CompletionSource Source) override;

		size_t GetCompletionUsingLine() override
		{
			return CompletionUsingLine;
		}

	private:
		std::vector<size_t> Changed;
		size_t CompletionUsingLine = 0;

		kui::Timer HoverTime;

		void* HoveredData = nullptr;
		kui::Vec2f LastCursorPosition;

		std::map<size_t, std::vector<ScriptSyntaxHighlight>> Highlights;
		void UpdateLineColorization(size_t Line);

		void UpdateFileContent();
		void UpdateFileData();
		void UpdateSyntaxHighlight();
	};
}