#pragma once
#include <kui/UI/FileEditorProvider.h>
#include <language.hpp>
#include <kui/Timer.h>

namespace engine::editor
{
	struct ScriptError
	{
		kui::EditorPosition At;
		size_t Length = 0;
		std::string Description;
	};

	struct ScriptSyntaxHighlight
	{
		size_t Start = 0;
		size_t Length = 0;
		kui::Vec3f Color;
	};

	class ScriptEditorProvider : public kui::FileEditorProvider
	{
	public:
		ScriptEditorProvider(std::string ScriptFile);

		std::string ScriptFile;
		lang::LanguageService* ScriptService = nullptr;
		std::vector<ScriptError> Errors;

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

	private:

		kui::Timer HoverTime;

		kui::UIBox* HoveredBox = nullptr;
		kui::Vec2f LastCursorPosition;

		std::map<size_t, std::vector<ScriptSyntaxHighlight>> Highlights;
		void UpdateLineColorization(size_t Line);

		void UpdateFile();
		void ScanFile();
	};
}