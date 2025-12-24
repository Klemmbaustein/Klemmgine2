#include "ScriptEditorContext.h"
#include <Engine/Engine.h>
#include <Engine/Script/ScriptSubsystem.h>
#include <Engine/MainThread.h>

using namespace ds;
using namespace kui;

engine::editor::ScriptEditorContext::ScriptEditorContext()
{
	this->ScriptService = Engine::GetSubsystem<script::ScriptSubsystem>()
		->ScriptLanguage->startService();

	this->ScriptService->parser->errors.errorCallback = [this]
	(ErrorCode code, std::string File, const Token& At, std::string Description)
	{
		this->NewErrors[File].push_back(ScriptError{
			.At = EditorPosition(At.position.startPos, At.position.line),
			.Length = At.position.endPos - At.position.startPos,
			.Description = Description,
			});
	};

	std::thread t = std::thread(&ScriptEditorContext::ContextCompilerThread, this);
	t.detach();
}

engine::editor::ScriptEditorContext::~ScriptEditorContext()
{
	Quit = true;
	cv.notify_all();
}

void engine::editor::ScriptEditorContext::AddFile(const std::string& Content, const std::string& Name)
{
	ScheduleContextTask([this, Content = Content, Name = Name] {
		this->ScriptService->addString(Content, Name);
	});
}

void engine::editor::ScriptEditorContext::UpdateFile(const std::string& Content, const std::string& Name)
{
	ScheduleContextTask([this, Content = Content, Name = Name] {
		NewErrors[Name].clear();
		ScriptService->updateFile(Content, Name);
	});
}

void engine::editor::ScriptEditorContext::Commit(std::function<void()> Callback)
{
	if (!Loaded)
	{
		return;
	}

	if (IsCompiling)
	{
		ReCompileNext = true;
		return;
	}

	ScheduleContextTask([this, Callback] {
		IsCompiling = true;
		ScriptService->commitChanges();
		if (Callback)
		{
			CallbackFunctions.push_back(Callback);
		}
		IsCompiling = false;
		thread::ExecuteOnMainThread([this, Callback] {
			Errors = NewErrors;

			if (ReCompileNext)
			{
				ReCompileNext = false;
				Commit(Callback);
			}
		});
	});
}

void engine::editor::ScriptEditorContext::ScheduleContextTask(std::function<void()> Task)
{
	std::lock_guard l{TaskReadMutex};
	Tasks.push(Task);
	cv.notify_one();
}

void engine::editor::ScriptEditorContext::Initialize()
{
	Loaded = true;
	Commit([this] {
		OnReady.Invoke();
	});
}

void engine::editor::ScriptEditorContext::ContextCompilerThread()
{
	while (!Quit)
	{
		std::unique_lock lk(TaskMutex);
		cv.wait(lk, [this] { return !Tasks.empty() || Quit; });

		TaskReadMutex.lock();
		while (!Tasks.empty())
		{
			std::function t = Tasks.front();
			Tasks.pop();
			TaskReadMutex.unlock();
			{
				std::lock_guard g{ ScriptServiceMutex };
				t();
			}
			TaskReadMutex.lock();
		}
		TaskReadMutex.unlock();

		for (auto& i : CallbackFunctions)
		{
			thread::ExecuteOnMainThread(i);
		}
		CallbackFunctions.clear();
	}
}
