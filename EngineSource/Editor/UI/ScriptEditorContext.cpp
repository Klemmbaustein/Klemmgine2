#include "ScriptEditorContext.h"
#include <Engine/Engine.h>
#include <Engine/Script/ScriptSubsystem.h>
#include <Engine/MainThread.h>
#include <Core/File/FileUtil.h>
#include <Engine/File/Resource.h>

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
	UpdateFilesList();
}

engine::editor::ScriptEditorContext::~ScriptEditorContext()
{
	Quit = true;
	cv.notify_all();

	std::unique_lock lk(ExitMutex);
	if (!Exited)
	{
		ExitCondition.wait(lk, [this] { return Exited; });
	}

	delete ScriptService;
}

void engine::editor::ScriptEditorContext::CompileUIFile(const std::string& Content, std::string Name, bool Update)
{
	script::ui::UIFileParser UIFiles;
	UIFiles.CompileScript = [this, Update](Token className, Token derivedName, string moduleName,
		ds::TokenStream& stream, string fileName) {
		if (Update)
		{
			return ScriptService->updateClass(className, moduleName, stream, fileName,
				{ { derivedName } });
		}
		return ScriptService->addClass(className, moduleName, stream, fileName,
			{ { derivedName } });
	};
	UIFiles.AddString(Name, Content);
	ParsedUI = UIFiles.Parse(ScriptService->parser);
}

void engine::editor::ScriptEditorContext::UpdateFilesList()
{
	ScheduleContextTask([this] {
		std::set<string> RemovedFiles = LoadedFiles;
		std::set<string> NewFiles;

		for (auto& [Name, Path] : resource::LoadedAssets)
		{
			string Extension = file::Extension(Name);
			if (Extension == "ds" || Extension == "kui")
			{
				if (RemovedFiles.contains(Path))
				{
					RemovedFiles.erase(Path);
				}
				else
				{
					NewFiles.insert(Path);
				}
			}
		}

		for (auto& Removed : RemovedFiles)
		{
			this->ScriptService->removeFile(Removed);
			LoadedFiles.erase(Removed);
		}

		for (auto& Added : NewFiles)
		{
			string Extension = file::Extension(Added);

			if (Extension != "kui")
			{
				this->ScriptService->addFile(resource::GetTextFile(Added), Added);
			}
			else
			{
				CompileUIFile(resource::GetTextFile(Added), Added, false);
			}
			LoadedFiles.insert(Added);
		}
	});
}

void engine::editor::ScriptEditorContext::AddFile(const string& Content, const string& Name)
{
	ScheduleContextTask([this, Content = Content, Name = Name] {

		if (file::Extension(Name) == "kui")
		{
			CompileUIFile(Content, Name, LoadedFiles.contains(Name));
		}
		else
		{
			if (LoadedFiles.contains(Name))
			{
				this->ScriptService->updateFile(Content, Name);
			}
			else
			{
				this->ScriptService->addFile(Content, Name);
				LoadedFiles.insert(Name);
			}
		}
	});
}

void engine::editor::ScriptEditorContext::UpdateFile(const string& Content, const string& Name)
{
	OnChange(Name);
	ScheduleContextTask([this, Content = Content, Name = Name] {
		NewErrors[Name].clear();
		if (file::Extension(Name) == "kui")
		{
			CompileUIFile(Content, Name, true);
		}
		else
		{
			this->ScriptService->updateFile(Content, Name);
		}
	});
}

void engine::editor::ScriptEditorContext::RemoveFile(const string& Name)
{
	UpdateFilesList();
}

std::vector<ds::AutoCompleteResult> engine::editor::ScriptEditorContext::CompleteAt(const string& FileName,
	size_t character, size_t line, ds::CompletionType type, size_t& OutUsingPosition)
{
	OutUsingPosition = 0;
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
			if (seg.second.Lines.size())
			{
				OutUsingPosition = seg.second.Lines[0].Strings[0].Line;
			}
			return ScriptService->completeAt(&ScriptService->files[FileName + "/" + f.FromToken.Text], character, line, type);
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
			PublishUIData(i, i.File);
		}

		if (Callback)
		{
			CallbackFunctions.push_back(Callback);
		}
		IsCompiling = false;
		if (Quit)
		{
			return;
		}
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
		var.kind = ScannedVariable::Kind::classMember;
		FileData.variables.push_back(var);
	}

	for (auto& i : For.Children)
	{
		FileData.types.push_back(Token(i.TypeName.Text,
			TokenPos(i.TypeName.BeginChar, i.TypeName.EndChar, i.TypeName.Line)));
		PublishUIData(i, File);
	}
}

void engine::editor::ScriptEditorContext::PublishUIData(kui::markup::MarkupElement& For, string File)
{
	string FromFileName = File + "/" + For.FromToken.Text;
	auto& FileData = ScriptService->files[FromFileName];
	auto& ToFile = ScriptService->files[File];

	for (auto& i : FileData.types)
	{
		ToFile.types.push_back(i);
	}

	for (auto& i : FileData.functions)
	{
		ToFile.functions.push_back(i);
	}

	for (auto& i : FileData.variables)
	{
		ToFile.variables.push_back(i);
	}

	for (auto& i : NewErrors[FromFileName])
	{
		NewErrors[File].push_back(i);
	}
	NewErrors[FromFileName].clear();

	PublishUIData(For.Root, File);
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

	std::unique_lock lk(ExitMutex);
	Exited = true;
	ExitCondition.notify_all();
}
