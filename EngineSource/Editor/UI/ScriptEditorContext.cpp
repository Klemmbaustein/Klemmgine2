#include "ScriptEditorContext.h"
#include <Engine/Engine.h>
#include <Engine/Script/ScriptSubsystem.h>
#include <Engine/MainThread.h>
#include <Core/File/FileUtil.h>

using namespace ds;
using namespace kui;

engine::editor::ScriptEditorContext::ScriptEditorContext()
{
	this->ScriptService = Engine::GetSubsystem<script::ScriptSubsystem>()
		->ScriptLanguage->startService();

	this->ScriptService->parser->errors.errorCallback = [this]
	(ErrorCode code, string File, const Token& At, string Description)
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

void engine::editor::ScriptEditorContext::AddFile(const string& Content, const string& Name)
{
	ScheduleContextTask([this, Content = Content, Name = Name] {

		if (file::Extension(Name) == "ds")
		{
			this->ScriptService->addString(Content, Name);
		}
		else
		{
			script::ui::UIFileParser UIFiles;
			UIFiles.CompileScript = [this](ds::Token className, string moduleName,
				ds::TokenStream& stream, string fileName) {
				return ScriptService->addClass(className, moduleName, stream, fileName,
					{ { ds::Token("engine::UIElement") } });
			};
			UIFiles.AddString(Name, Content);
			ParsedUI = UIFiles.Parse(ScriptService->parser);
		}
	});
}

void engine::editor::ScriptEditorContext::UpdateFile(const string& Content, const string& Name)
{
	ScheduleContextTask([this, Content = Content, Name = Name] {
		NewErrors[Name].clear();
		if (file::Extension(Name) == "kui")
		{
			script::ui::UIFileParser UIFiles;
			UIFiles.CompileScript = [this](ds::Token className, string moduleName,
				ds::TokenStream& stream, string fileName) {
				return ScriptService->updateClass(className, moduleName, stream, fileName,
					{ { ds::Token("engine::UIElement") } });
			};
			UIFiles.AddString(Name, Content);
			ParsedUI = UIFiles.Parse(ScriptService->parser);
		}
		else
		{
			this->ScriptService->updateFile(Content, Name);
		}
	});
}

std::vector<ds::AutoCompleteResult> engine::editor::ScriptEditorContext::CompleteAt(const string& FileName,
	size_t character, size_t line, ds::CompletionType type)
{
	if (file::Extension(FileName) != "kui")
	{
		return ScriptService->completeAt(&ScriptService->files[FileName], character, line, type);
	}

	std::function CheckElement = [character, line](kui::markup::UIElement& Elem)
		-> std::optional<std::vector<ds::AutoCompleteResult>> {
		kui::markup::UIElement* Current = &Elem;

		bool FoundChild = false;

		do
		{
			FoundChild = false;
			for (auto& seg : Current->Children)
			{
				if (line < seg.StartLine || line > seg.EndLine)
				{
					continue;
				}

				Current = &seg;
				FoundChild = true;
				break;
			}
		} while (FoundChild);

		if (line < Current->StartLine || line > Current->EndLine)
		{
			return {};
		}

		kui::markup::PropElementType FoundType = kui::markup::GetTypeFromString(Current->TypeName);
		std::set<string> AutoCompleteValues;

		auto Completions = std::vector{
			ds::AutoCompleteResult("child"),
			ds::AutoCompleteResult("var"),
			ds::AutoCompleteResult("true"),
			ds::AutoCompleteResult("false"),
			ds::AutoCompleteResult("horizontal"),
			ds::AutoCompleteResult("vertical"),
			ds::AutoCompleteResult("default"),
			ds::AutoCompleteResult("centered"),
			ds::AutoCompleteResult("reverse"),
		};

		for (const auto& i : kui::markup::Properties)
		{
			if (!IsSubclassOf(FoundType, i.Type))
			{
				continue;
			}
			if (AutoCompleteValues.contains(i.Name))
				continue;
			AutoCompleteValues.insert(i.Name);

			Completions.push_back(ds::AutoCompleteResult(i.Name));
		}

		return Completions;
	};

	for (auto& f : this->ParsedUI.UIData.Elements)
	{
		if (f.File != FileName)
		{
			continue;
		}

		for (auto& seg : f.CustomSegments)
		{
			if (line < seg.second.StartLine || line > seg.second.EndLine)
			{
				continue;
			}
			return ScriptService->completeAt(&ScriptService->files[FileName], character, line, type);
		}

		auto Completion = CheckElement(f.Root);

		if (Completion)
		{
			return *Completion;
		}
	}
	return {
		ds::AutoCompleteResult("element"),
		ds::AutoCompleteResult("global"),
		ds::AutoCompleteResult("const"),
	};
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

		for (auto& i : this->ParsedUI.UIData.Elements)
		{
			PublishUIData(i.Root, i.File);
		}

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

void engine::editor::ScriptEditorContext::PublishUIData(kui::markup::UIElement& For, string File)
{
	auto& FileData = ScriptService->files[File];

	for (auto& i : For.ElementProperties)
	{
		ScannedVariable var;
		var.at = Token(i.Name.Text, TokenPos(i.Name.BeginChar, i.Name.EndChar, i.Name.Line));
		var.name = i.Name.Text;
		var.defaultValue = i.Value.Text;
		var.inClass = For.TypeName.Text;
		FileData.variables.push_back(var);
	}

	for (auto& i : For.Children)
	{
		PublishUIData(i, File);
	}
}

void engine::editor::ScriptEditorContext::ScheduleContextTask(std::function<void()> Task)
{
	std::lock_guard l{ TaskReadMutex };
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
