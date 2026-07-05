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
#include <Engine/Subsystem/ConsoleSubsystem.h>
#include <Engine/UI/UICanvas.h>

#if EDITOR
#include <Editor/UI/Panels/PropertyPanel.h>
#include <Editor/UI/EditorUI.h>
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
		.useJustInTimeCompiler = launchArgs::GetArg("useJIT").has_value(),
		});
	Runtime->createBackgroundThread = [](std::function<void()> function) {
		ThreadPool::Main()->AddJob(function);
	};

	this->Runtime->writeError = [this](const char* Message) {
		Print(Message, LogType::Error);
	};

	Reload();
}

engine::script::ScriptSubsystem::~ScriptSubsystem()
{
	for (auto& [ClassId, ObjectId] : ScriptObjectIds)
	{
		Reflection::UnRegisterObject(ObjectId);
	}

	// Check for memory leaks. Ideally if the reference counting works as expected this should be 0.
	Print("Script classes leaked: " + std::to_string(RuntimeClass::classRefCount), LogType::Note);

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
	ThreadPool::Main()->AwaitJoin();
	auto CurrentScene = Scene::GetMain();

	auto Compiler = this->ScriptLanguage->createCompiler(ParserOptions{
		.printAssembly = launchArgs::GetArg("printScriptAssembly").has_value()
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

	UICanvas::ClearAll();

	BeginHotReloadEvent.Invoke();

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

		if (IsSceneObject)
		{
			ScriptObjectIds[Id] = Reflection::RegisterObject(Name, [TypeInfo = TypeInfo, this]() {
				return new ScriptSceneObject(TypeInfo, this->Runtime->baseContext);
			}, str::Hash("Engine/SceneObject"), Path);
		}
		else
		{
			ScriptObjectIds[Id] = Reflection::RegisterObject(Name, [TypeInfo = TypeInfo, this]() {
				auto NewManager = new ScriptSceneManager(TypeInfo, this->Runtime->baseContext);
				NewManager->InitializeScriptPointer();

				return NewManager;
			}, str::Hash("Engine/Scene/SceneManager"), Path);
		}
	}

#if EDITOR

	editor::EditorUI::ForEachPanel<editor::PropertyPanel>([](editor::PropertyPanel* p) {
		p->OnResized();
	});

#endif

	return true;
}

RuntimeClass* engine::script::ScriptSubsystem::GetClassFromObject(ReflectionObject* Object)
{
	auto found = ScriptObjectMappings.find(Object);

	if (found != ScriptObjectMappings.end())
	{
		found->second->addRef();
		return found->second;
	}

	RuntimeClass* NewObj = CreateSceneObject(Object);
	NewObj->addRef();
	RegisterClassForObject(Object, NewObj);
	return NewObj;
}

void engine::script::ScriptSubsystem::RegisterClassForObject(ReflectionObject* Object, ds::RuntimeClass* Class)
{
	if (!Class)
	{
		return;
	}

	ScriptObjectMappings.insert({ Object, Class });
	Object->OnDestroyedEvent.Add(this, [this, Object, Class]() {
		if (Class)
		{
			*(void**)Class->getBody() = nullptr;
			Runtime->baseContext->destruct(Class);
			ScriptObjectMappings.erase(Object);
		}
		Object->OnDestroyedEvent.Remove(this);
	});
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
