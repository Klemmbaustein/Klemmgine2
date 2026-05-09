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
#include <Engine/Script/FileScriptProvider.h>
#include <Engine/Script/ScriptSceneObject.h>
#include <Engine/Script/ScriptSceneManager.h>
#include <Engine/Script/ScriptSerializer.h>
#include <Engine/Script/UI/ScriptUIElement.h>
#include <Engine/Stats.h>
#include <Engine/Subsystem/ConsoleSubsystem.h>
#include <Engine/UI/UICanvas.h>

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
		.useJustInTimeCompiler = !launchArgs::GetArg("noJIT").has_value(),
		});
	Runtime->createBackgroundThread = [](std::function<void()> function) {
		ThreadPool::Main()->AddJob(function);
	};

	this->Runtime->writeError = [this](const char* Message) {
		Print(Message, LogType::Error);
	};

	Scripts = new FileScriptProvider();

	Reload();
}

engine::script::ScriptSubsystem::~ScriptSubsystem()
{
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

	//std::cout << "stk: " << Runtime->baseContext->stackPos << std::endl;

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
	if (NewInstructions.code.empty())
	{
		if (resource::FileExists("scriptCache.bin"))
		{
			auto Stream = FileStream("scriptCache.bin", true);
			script::serialize::DeSerializeBytecode(&NewInstructions, &UIData, &Stream);
		}
		else
		{
			return false;
		}
	}
	else
	{
		auto Stream = FileStream("scriptCache.bin", false);
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
		string Path = TypeInfo.name.substr(0, LastColon - 1);

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

	RuntimeClass* NewObj = NativeModule::makePointerClass<ReflectionObject>(Object);
	NewObj->addRef();
	RegisterClassForObject(Object, NewObj);
	return NewObj;
}

void engine::script::ScriptSubsystem::RegisterClassForObject(ReflectionObject* Object, ds::RuntimeClass* Class)
{
	ScriptObjectMappings.insert({ Object, Class });
	Object->OnDestroyedEvent.Add(this, [this, Object, Class]() {
		*(void**)Class->getBody() = nullptr;
		Runtime->baseContext->destruct(Class);
		ScriptObjectMappings.erase(Object);
	});
}

void engine::script::ScriptSubsystem::ReloadDynamicUIContext()
{
	UIContext.Parsed = &UIData.UIData;

	this->UIContext.CreateSpecialMarkupBox.clear();

	for (auto& i : this->UIData.ClassIdMappings)
	{
		this->UIContext.CreateSpecialMarkupBox[i.second] =
			[this, cls = i.first](kui::markup::DynamicMarkupContext* c) -> kui::markup::UIDynMarkupBox* {
			auto Class = Runtime->reflect->types[cls].create(Runtime->baseContext);

			ClassRef<ui::ScriptUIElement*> Element = Class;

			return Element.getValue();
		};
	}
}
