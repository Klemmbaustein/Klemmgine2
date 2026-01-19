#pragma once
#include <ds/language.hpp>
#include <kui/UI/TextEditor.h>
#include <Core/Event.h>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <Engine/Script/UI/ParseUI.h>

namespace engine::editor
{
	struct ScriptError
	{
		kui::EditorPosition At;
		size_t Length = 0;
		string Description;
	};

	class ScriptEditorContext
	{
	public:
		ds::LanguageService* ScriptService = nullptr;
		std::map<string, std::vector<ScriptError>> Errors;

		ScriptEditorContext();
		ScriptEditorContext(const ScriptEditorContext&) = delete;
		virtual ~ScriptEditorContext();

		void AddFile(const string& Content, const string& Name);
		void UpdateFile(const string& Content, const string& Name);
		void Commit(std::function<void()> Callback);

		void Initialize();

		bool Loaded = false;

		std::mutex ScriptServiceMutex;

		Event<> OnReady;

		virtual void NavigateTo(std::string File, ds::TokenPos at) = 0;

		std::vector<ds::AutoCompleteResult> CompleteAt(const string& FileName, size_t character, size_t line,
			ds::CompletionType type);

	private:
		bool SendUpdateEvent = false;
		bool IsCompiling = false;
		bool ReCompileNext = false;
		bool Quit = false;

		std::condition_variable cv;
		std::mutex TaskMutex;
		std::mutex TaskReadMutex;
		std::map<string, std::vector<ScriptError>> NewErrors;
		script::ui::UIParseData ParsedUI;

		void ScheduleContextTask(std::function<void()> Task);

		void PublishUIData(kui::markup::UIElement& For, string File);

		std::queue<std::function<void()>> Tasks;

		void ContextCompilerThread();

		std::vector<std::function<void()>> CallbackFunctions;
	};
}