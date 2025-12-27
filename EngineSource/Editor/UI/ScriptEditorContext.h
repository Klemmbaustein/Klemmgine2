#pragma once
#include <ds/language.hpp>
#include <kui/UI/TextEditor.h>
#include <Core/Event.h>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>

namespace engine::editor
{
	struct ScriptError
	{
		kui::EditorPosition At;
		size_t Length = 0;
		std::string Description;
	};

	class ScriptEditorContext
	{
	public:
		ds::LanguageService* ScriptService = nullptr;
		std::map<std::string, std::vector<ScriptError>> Errors;

		ScriptEditorContext();
		ScriptEditorContext(const ScriptEditorContext&) = delete;
		virtual ~ScriptEditorContext();

		void AddFile(const std::string& Content, const std::string& Name);
		void UpdateFile(const std::string& Content, const std::string& Name);
		void Commit(std::function<void()> Callback);

		void Initialize();

		bool Loaded = false;

		std::mutex ScriptServiceMutex;

		Event<> OnReady;

		virtual void NavigateTo(std::string File, ds::TokenPos at) = 0;

	private:
		bool SendUpdateEvent = false;
		bool IsCompiling = false;
		bool ReCompileNext = false;
		bool Quit = false;

		std::condition_variable cv;
		std::mutex TaskMutex;
		std::mutex TaskReadMutex;
		std::map<std::string, std::vector<ScriptError>> NewErrors;

		void ScheduleContextTask(std::function<void()> Task);

		std::queue<std::function<void()>> Tasks;

		void ContextCompilerThread();

		std::vector<std::function<void()>> CallbackFunctions;
	};
}