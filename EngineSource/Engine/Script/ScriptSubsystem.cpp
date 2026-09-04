#include "ScriptSubsystem.h"
#include <Core/Error/EngineAssert.h>
#include <Core/File/FileUtil.h>
#include <Core/LaunchArgs.h>
#include <Core/ThreadPool.h>
#include <ds/language.hpp>
#include <ds/modules/standardLibrary.hpp>
#include <ds/modules/system.async.hpp>
#include <Engine/Debug/TimeLogger.h>
#include <Engine/File/Resource.h>
#include <Engine/Scene.h>
#include <Engine/Script/ScriptSceneObject.h>
#include <Engine/Script/ScriptSceneManager.h>
#include <Engine/Script/ScriptSerializer.h>
#include <Engine/Script/UI/ScriptUIElement.h>
#include <Engine/Stats.h>
#include <Engine/Engine.h>
#include <Engine/Subsystem/ConsoleSubsystem.h>
#include <Engine/UI/UICanvas.h>
#include <Engine/Subsystem/InputSubsystem.h>

#if EDITOR
#include <Editor/UI/Panels/PropertyPanel.h>
#include <Editor/UI/EditorUI.h>
#include <Editor/Assets/ScriptAssetType.h>
#include <Editor/EditorSubsystem.h>
#endif
#include <Engine/ProjectFile.h>

using namespace ds;
using namespace ds::modules::system::async;

engine::script::ScriptSubsystem* engine::script::ScriptSubsystem::Instance = nullptr;

engine::script::ScriptSubsystem::ScriptSubsystem()
	: subsystem::Subsystem("Script", Log::LogColor::Yellow)
{
	Instance = this;

	this->ScriptLanguage = new LanguageContext();
	modules::registerStandardLibrary(this->ScriptLanguage);
	ScriptEngine = RegisterEngineModules(this->ScriptLanguage);

	ScriptInstructions = new BytecodeStream();
	Runtime = this->ScriptLanguage->createRuntime({
		.useJustInTimeCompiler = launchArgs::GetArg("useJIT").has_value() || Engine::Instance->OpenedProject->UseScriptJIT,
		});
	Runtime->createBackgroundThread = [](std::function<void()> function) {
		ThreadPool::Main()->AddJob(function);
	};

	Runtime->writeError = [this](const char* Message) {
		Print(Message, LogType::Error);
	};

	Runtime->onDebugBreak = [this](InterpretContext* context, Pointer bytecodePosition, DebugState* state) {

		IsOnBreakpoint = true;

		for (auto& [Line, File] : BreakpointLines)
		{
			if (Line.offset != bytecodePosition)
			{
				continue;
			}
			Log::Info(str::Format("Hit breakpoint: %s:%i", File.c_str(), Line.lineNumber));

			auto ScriptAsset = dynamic_cast<editor::ScriptAssetType*>(
				editor::EditorUI::Instance->GetAssetTypeForExtension("ds"));

			if (ScriptAsset)
			{
				ScriptAsset->RunOnActiveScriptEditor([File = File, Line = Line.lineNumber]
				(editor::ScriptEditorUI* UI) {
					UI->HighlightLine(File, Line);
				});
			}
		}

		auto frames = state->getFrames();

		for (auto& i : frames)
		{
			Log::Info(context->runtime->debug->getSectionAt(i->getOffset())->name);

			auto variables = i->getVariables();
			for (auto& j : variables)
			{
				Log::Info(str::Format("\t%s", j.name));
			}
		}

		auto& w = VideoSubsystem::Current->MainWindow;

		auto Input = Engine::GetSubsystem<subsystem::InputSubsystem>();
		auto OldShowCursor = input::ShowMouseCursor;
		input::ShowMouseCursor = true;
		Engine::GameHasFocus = false;
		Engine::IsPaused = true;

		while (IsOnBreakpoint && !Engine::Instance->ShouldQuit)
		{
			thread::MainThreadUpdate();
			Input->Update();
			Engine::GetSubsystem<editor::EditorSubsystem>()->Update();
			VideoSubsystem::Current->Update();
			VideoSubsystem::Current->RenderUpdate();
		}
		Engine::IsPaused = false;
	};

	Reload();
}

engine::script::ScriptSubsystem::~ScriptSubsystem()
{
	ClearTasks();
	for (auto& [ClassId, ObjectId] : ScriptObjectIds)
	{
		Reflection::UnRegisterObject(ObjectId);
	}

	// Check for memory leaks. Ideally if the reference counting works as expected this should be 0.
	Print("Script classes leaked: " + std::to_string(RuntimeClass::classRefCount), LogType::Note);
	Print("Object map leaks: " + std::to_string(ScriptObjectMappings.size()), LogType::Note);

	delete this->Runtime;
	delete this->ScriptLanguage;
}

void engine::script::ScriptSubsystem::RegisterCommands(ConsoleSubsystem* System)
{
	System->AddCommand(console::Command{
		.Name = "script.reload",
		.Args = {},
		.OnCalled = [this](const console::Command::CallContext&) {
			this->Reload();
		}
		});

	System->AddCommand(console::Command{
		.Name = "script.breakpoint",
		.Args = {console::Command::Argument("file", true), console::Command::Argument("line", true)},
		.OnCalled = [this](const console::Command::CallContext& c) {

			string File = c.ProvidedArguments[0];
			int32 Line = std::stoi(c.ProvidedArguments[1]);

			AddBreakpoint(File, Line);
		}
		});
	System->AddCommand(console::Command{
		.Name = "script.continue",
		.Args = {},
		.OnCalled = [this](const console::Command::CallContext& c) {
			ScriptSubsystem::Instance->IsOnBreakpoint = false;
		}
		});
}

void engine::script::ScriptSubsystem::AddBreakpoint(string File, size_t Line)
{
	this->Breakpoints[File].insert(Line);
	ApplyBreakpoint(File, Line);
}

void engine::script::ScriptSubsystem::ApplyBreakpoint(string File, size_t Line)
{
	auto FoundLine = this->Runtime->debug->getLineAt(File, Line);

	if (FoundLine && this->Runtime->baseContext->setDebugBreakpoint(FoundLine->offset))
	{
		BreakpointLines.push_back({ *FoundLine, File });
		Log::Info(str::Format("Set breakpoint at %s:%i -> instructions+%i", File.c_str(), Line, FoundLine->offset));
	}
	else
	{
		Log::Warn(str::Format("Failed to set breakpoint at %s:%i", File.c_str(), Line));
	}
}

void engine::script::ScriptSubsystem::Update()
{
	std::vector<std::list<WaitTask>::iterator> ToRemove;

	for (auto it = WaitTasks.begin(); it != WaitTasks.end(); it++)
	{
		it->Time -= stats::DeltaTime;

		if (it->Time <= 0)
		{
			completeTask(it->TaskObject, this->Runtime->baseContext);
			this->Runtime->baseContext->destruct(it->TaskObject);
			ToRemove.push_back(it);
		}
	}

	for (auto& i : ToRemove)
	{
		WaitTasks.erase(i);
	}
}

bool engine::script::ScriptSubsystem::Reload()
{
	ClearTasks();
	ThreadPool::Main()->AwaitJoin();
	auto CurrentScene = Scene::GetMain();

	auto Compiler = this->ScriptLanguage->createCompiler(ParserOptions{
		.printAssembly = launchArgs::GetArg("printScriptAssembly").has_value(),
		});

	debug::TimeLogger CompileTime = { "Compiled scripts", this->GetLogPrefixes() };

	Compiler->errors.writeError = [this](string Message) {
		Print(Message, LogType::Error);
	};

	ui::UIFileParser UIFiles = ui::UIFileParser();

	for (const auto& [Name, Path] : resource::LoadedAssets)
	{
		string Extension = file::Extension(Name);

		if (Extension == "ds")
		{
			Compiler->addString(resource::GetTextFile(Path), Path);
		}
		else if (Extension == "kui")
		{
			UIFiles.AddString(Path, resource::GetTextFile(Path));
		}
	}

	UIData = UIFiles.Parse(Compiler);

	auto NewInstructions = Compiler->compile();
	UIFiles.OnCompileFinished(UIData);
	delete Compiler;

#ifdef EDITOR
	string ScriptCachePath = editor::EditorUI::Instance->GetProjectDataPath() + "/scriptCache.bin";

	if (NewInstructions.code.empty())
	{
		if (resource::FileExists(ScriptCachePath))
		{
			auto Stream = FileStream(ScriptCachePath, true);
			script::serialize::DeSerializeBytecode(&NewInstructions, &UIData, &Stream);
		}
		else
		{
			return false;
		}
	}
	else
	{
		auto Stream = FileStream(ScriptCachePath, false);
		script::serialize::SerializeBytecode(&NewInstructions, &UIData, &Stream);
	}
#else
	if (NewInstructions.code.empty())
	{
		return false;
	}
#endif

	if (!DoingHotReload)
	{
		UICanvas::ClearAll();

		BeginHotReloadEvent.Invoke();
	}
	DoingHotReload = false;
	*ScriptInstructions = NewInstructions;
	this->Runtime->loadBytecode(ScriptInstructions);
	ReloadDynamicUIContext();

	EndHotReloadEvent.Invoke();

	for (auto& [ClassId, ObjectId] : ScriptObjectIds)
	{
		Reflection::UnRegisterObject(ObjectId);
	}

	for (auto& [Id, TypeInfo] : ScriptInstructions->reflect.types)
	{
		bool IsSceneObject = ScriptInstructions->reflect.isSubclassOf(Id, ScriptEngine.SceneObjectType);
		bool IsSceneManager = ScriptInstructions->reflect.isSubclassOf(Id, ScriptEngine.SceneManagerType);

		if (!IsSceneObject && !IsSceneManager)
		{
			continue;
		}

		size_t LastColon = TypeInfo.name.find_last_of(':');

		string Name = TypeInfo.name.substr(LastColon + 1);
		string Path = LastColon == string::npos ? "" : TypeInfo.name.substr(0, LastColon - 1);

		ObjectTypeID ObjectId = 0;

		if (IsSceneObject)
		{
			ObjectId = Reflection::RegisterObject(Name, [TypeInfo = TypeInfo, this]() {
				return new ScriptSceneObject(TypeInfo, this->Runtime->baseContext);
			}, str::Hash("Engine/SceneObject"), Path);
		}
		else
		{
			ObjectId = Reflection::RegisterObject(Name, [TypeInfo = TypeInfo, this]() {
				auto NewManager = new ScriptSceneManager(TypeInfo, this->Runtime->baseContext);
				NewManager->InitializeScriptPointer();
				return NewManager;
			}, str::Hash("Engine/Scene/SceneManager"), Path);
		}

#if EDITOR
		for (auto& i : TypeInfo.attributes)
		{
			if (i.type == this->ScriptEngine.IconAttributeType)
			{
				auto val = i.getParameterValue("name");
				if (!val)
				{
					continue;
				}

				AssetRef Asset = AssetRef::Convert(*val);

				if (Asset.Exists())
				{
					editor::EditorUI::ObjectIcons.AddObjectIcon(Asset.FilePath, ObjectId);
				}
			}
		}
#endif
		ScriptObjectIds[Id] = ObjectId;
	}

	ReInitializeAfterHotReloadEvent.Invoke();

#if EDITOR

	editor::EditorUI::ForEachPanel<editor::PropertyPanel>([](editor::PropertyPanel* p) {
		p->OnResized();
	});

#endif

	return true;
}

void engine::script::ScriptSubsystem::ClearTasks()
{
	for (auto& i : WaitTasks)
	{
		modules::system::async::abortTask(i.TaskObject, this->Runtime->baseContext);
		this->Runtime->baseContext->destruct(i.TaskObject);
	}

	WaitTasks.clear();
}

void engine::script::ScriptSubsystem::RegisterClassForObject(Destructible* Object, ds::RuntimeClass* Class, bool Destruct)
{
	if (!Class)
	{
		return;
	}

	ScriptObjectMappings[Object] = Class;
	Object->OnDestroyedEvent.Add(Class, [this, Object, Class, Destruct]() {
		if (Class)
		{
			*(void**)Class->getBody() = nullptr;
			if (Destruct)
			{
				Runtime->baseContext->destruct(Class);
			}
			ScriptObjectMappings.erase(Object);
		}
		Object->OnDestroyedEvent.Remove(Class);
	});
}

void engine::script::ScriptSubsystem::ReloadRuntime()
{
	ClearTasks();
	UICanvas::ClearAll();
	BeginHotReloadEvent.Invoke();
	DoingHotReload = true;
	delete this->Runtime;

	this->Runtime = Runtime = this->ScriptLanguage->createRuntime({
		.useJustInTimeCompiler = launchArgs::GetArg("useJIT").has_value() || Engine::Instance->OpenedProject->UseScriptJIT,
		});

	Reload();
}

void engine::script::ScriptSubsystem::ReloadDynamicUIContext()
{
	UIContext.Parsed = &UIData.UIData;

	this->UIContext.CreateSpecialMarkupBox.clear();

	for (auto& i : this->UIData.ClassIdMappings)
	{
		this->UIContext.CreateSpecialMarkupBox[i.second] =
			[this, cls = Runtime->reflect->types.at(i.first)]
			(kui::markup::DynamicMarkupContext* c, bool HasName) -> kui::markup::UIDynMarkupBox* {
			if (!HasName)
			{
				return nullptr;
			}

			auto found = UIObjectMappings.find(c);

			if (found != UIObjectMappings.end())
			{
				ClassRef<ui::ScriptUIElement*> Element = found->second;
				return Element.getValue();
			}

			auto Class = cls.create(Runtime->baseContext);

			ClassRef<ui::ScriptUIElement*> Element = Class;

			return Element.getValue();
		};
	}
}
