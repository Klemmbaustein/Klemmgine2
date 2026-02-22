#include "ScriptSubsystem.h"
#include "ScriptObject.h"
#include "FileScriptProvider.h"
#include <Engine/File/Resource.h>
#include <Engine/Stats.h>
#include <Engine/Subsystem/ConsoleSubsystem.h>
#include <ds/language.hpp>
#include <ds/modules/standardLibrary.hpp>
#include <Engine/Debug/TimeLogger.h>
#include <Engine/Scene.h>
#include <Core/File/FileUtil.h>
#include <Core/ThreadPool.h>
#include <ds/modules/system.async.hpp>
#include <Engine/UI/UICanvas.h>
#include <Engine/Script/UI/ScriptUIElement.h>
#include <Engine/Script/ScriptSerializer.h>

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
	Runtime = this->ScriptLanguage->createRuntime();
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
			completeTask(it->TaskObject, &this->Runtime->baseContext);
			this->Runtime->baseContext.destruct(it->TaskObject);
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

	auto Compiler = this->ScriptLanguage->createCompiler();

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

	if (NewInstructions.code.empty())
	{
		return false;
	}

	if (CurrentScene)
	{
		UICanvas::ClearAll();
		for (auto& i : CurrentScene->Objects)
		{
			auto ScriptObj = dynamic_cast<ScriptObject*>(i);
			if (ScriptObj)
			{
				ScriptObj->OnDestroyed();
				ScriptObj->OnDestroyedEvent.Invoke();
				ScriptObj->ClearComponents();
			}
		}
	}

	//auto Stream = FileStream("scripts.bin", true);
	//script::serialize::SerializeBytecode(&NewInstructions, &UIData, &Stream);

	*ScriptInstructions = NewInstructions;
	//script::serialize::DeSerializeBytecode(ScriptInstructions, &UIData, &Stream);
	this->Runtime->loadBytecode(ScriptInstructions);
	ReloadDynamicContext();

	if (CurrentScene)
	{
		for (auto& i : CurrentScene->Objects)
		{
			auto ScriptObj = dynamic_cast<ScriptObject*>(i);
			if (ScriptObj)
			{
				ScriptObj->Class = ScriptInstructions->reflect.types[ScriptObj->Class.hash];
				ScriptObj->LoadScriptData();
				ScriptObj->Begin();
			}
		}
	}

	for (auto& [ClassId, ObjectId] : ScriptObjectIds)
	{
		Reflection::UnRegisterObject(ObjectId);
	}

	for (auto& [Id, TypeInfo] : ScriptInstructions->reflect.types)
	{
		if (!ScriptInstructions->reflect.isSubclassOf(Id, ScriptEngine.ScriptObjectType))
		{
			continue;
		}

		size_t LastColon = TypeInfo.name.find_last_of(':');

		string Name = TypeInfo.name.substr(LastColon + 1);
		string Path = TypeInfo.name.substr(0, LastColon - 1);

		ScriptObjectIds[Id] = Reflection::RegisterObject(Name, [TypeInfo = TypeInfo, this]() -> SceneObject* {
			return new ScriptObject(TypeInfo, &this->Runtime->baseContext);
		}, Path);
	}
	return true;
}

RuntimeClass* engine::script::ScriptSubsystem::GetClassFromObject(SceneObject* Object)
{
	auto found = ScriptObjectMappings.find(Object);

	if (found != ScriptObjectMappings.end())
	{
		found->second->addRef();
		return found->second;
	}

	RuntimeClass* NewObj = NativeModule::makePointerClass<SceneObject>(Object);
	NewObj->addRef();
	RegisterClassForObject(Object, NewObj);
	return NewObj;
}

void engine::script::ScriptSubsystem::RegisterClassForObject(SceneObject* Object, ds::RuntimeClass* Class)
{
	ScriptObjectMappings.insert({ Object, Class });
	Object->OnDestroyedEvent.Add(this, [this, Object, Class]() {
		*(void**)Class->getBody() = nullptr;
		Runtime->baseContext.destruct(Class);
		ScriptObjectMappings.erase(Object);
	});
}

void engine::script::ScriptSubsystem::ReloadDynamicContext()
{
	UIContext.Parsed = &UIData.UIData;

	this->UIContext.CreateSpecialMarkupBox.clear();

	for (auto& i : this->UIData.ClassIdMappings)
	{
		this->UIContext.CreateSpecialMarkupBox[i.second] =
			[this, cls = i.first](kui::markup::DynamicMarkupContext* c) -> kui::markup::UIDynMarkupBox* {
			auto Class = Runtime->reflect->types[cls].create(&Runtime->baseContext);

			ClassRef<ui::ScriptUIElement*> Element = Class;

			return Element.getValue();
		};
	}
}
