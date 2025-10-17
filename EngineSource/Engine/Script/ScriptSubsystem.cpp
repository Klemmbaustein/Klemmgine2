#include "ScriptSubsystem.h"
#include "ScriptObject.h"
#include "EngineModules.h"
#include "FileScriptProvider.h"
#include "ScriptSerializer.h"
#include <Engine/Stats.h>
#include <Engine/Subsystem/ConsoleSubsystem.h>
#include <ds/language.hpp>
#include <ds/modules/standardLibrary.hpp>
#include <Engine/Debug/TimeLogger.h>
#include <Engine/Scene.h>
#include <Core/ThreadPool.h>
#include <filesystem>
#include <ds/modules/system.async.hpp>

using namespace ds::modules::system::async;
using namespace ds;

engine::script::ScriptSubsystem::ScriptSubsystem()
	: subsystem::Subsystem("Script", Log::LogColor::Yellow)
{
	this->ScriptLanguage = new LanguageContext();
	modules::registerStandardLibrary(this->ScriptLanguage);
	RegisterEngineModules(this->ScriptLanguage);

	ScriptInstructions = new BytecodeStream();
	Runtime = this->ScriptLanguage->createRuntime();
	Runtime->createBackgroundThread = [](std::function<void()> function) {
		ThreadPool::Main()->AddJob(function);
	};

	Scripts = new FileScriptProvider();

	Reload();
}

engine::script::ScriptSubsystem::~ScriptSubsystem()
{
	Log::Info(std::to_string(RuntimeClass::classRefCount));

	delete this->Runtime;
	delete this->ScriptLanguage;
}

void engine::script::ScriptSubsystem::RegisterCommands(subsystem::ConsoleSubsystem* System)
{
	System->AddCommand(console::Command{
		.Name = "script_reload",
		.Args = {},
		.OnCalled = [this](const console::Command::CallContext&)
		{
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

void engine::script::ScriptSubsystem::Reload()
{
	ThreadPool::Main()->AwaitJoin();
	auto CurrentScene = Scene::GetMain();

	auto Compiler = this->ScriptLanguage->createCompiler();

	debug::TimeLogger CompileTime = { "Compiled scripts", this->GetLogPrefixes() };

	Compiler->errors.writeError = [this](string Message)
	{
		Print(Message, LogType::Error);
	};

	for (auto& i : Scripts->GetFiles())
	{
		Compiler->addString(Scripts->GetFile(i), i);
	}

	auto NewInstructions = Compiler->compile();;
	delete Compiler;

	if (NewInstructions.code.empty())
	{
		return;
	}

	if (CurrentScene)
	{
		for (auto& i : CurrentScene->Objects)
		{
			auto ScriptObj = dynamic_cast<ScriptObject*>(i);
			if (ScriptObj)
			{
				ScriptObj->UnloadScriptData();
			}
		}
	}
	//auto Stream = FileStream("scripts.bin", true);
	//script::serialize::SerializeBytecode(&NewInstructions, &Stream);

	*ScriptInstructions = NewInstructions;
	//script::serialize::DeSerializeBytecode(ScriptInstructions, &Stream);
	this->Runtime->loadBytecode(ScriptInstructions);

	if (CurrentScene)
	{
		for (auto& i : CurrentScene->Objects)
		{
			auto ScriptObj = dynamic_cast<ScriptObject*>(i);
			if (ScriptObj)
			{
				ScriptObj->Class = ScriptInstructions->reflect.types[ScriptObj->Class.hash];
				ScriptObj->LoadScriptData();
			}
		}
	}

	for (auto& [Id, TypeInfo] : ScriptInstructions->reflect.types)
	{
		Reflection::RegisterObject(TypeInfo.name, [&TypeInfo, this] -> SceneObject* {
			return new ScriptObject(TypeInfo, &this->Runtime->baseContext);
		});
	}
}
