#include "ScriptSubsystem.h"
#include "ScriptObject.h"
#include "EngineModules.h"
#include "ScriptSerializer.h"
#include <Engine/Subsystem/ConsoleSubsystem.h>
#include <ds/language.hpp>
#include <ds/modules/standardLibrary.hpp>
#include <Engine/Debug/TimeLogger.h>
#include <Engine/Scene.h>
#include <filesystem>

using namespace ds;

engine::script::ScriptSubsystem::ScriptSubsystem()
	: subsystem::Subsystem("Script", Log::LogColor::Yellow)
{
	this->ScriptLanguage = new LanguageContext();
	modules::registerStandardLibrary(this->ScriptLanguage);
	RegisterEngineModules(this->ScriptLanguage);

	ScriptInstructions = new BytecodeStream();
	Interpreter = this->ScriptLanguage->createInterpreter();

	Reload();
}

engine::script::ScriptSubsystem::~ScriptSubsystem()
{
	delete this->Interpreter;
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

void engine::script::ScriptSubsystem::Reload()
{
	auto CurrentScene = Scene::GetMain();

	auto Compiler = this->ScriptLanguage->createCompiler();

	debug::TimeLogger CompileTime = { "Compiled scripts", this->GetLogPrefixes() };

	Compiler->errors.writeError = [this](string Message)
	{
		Print(Message, LogType::Error);
	};

	if (std::filesystem::is_directory("Scripts"))
	{
		for (auto& i : std::filesystem::recursive_directory_iterator("Scripts"))
		{
			Compiler->addFile(i.path().string());
		}
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
	this->Interpreter->loadBytecode(ScriptInstructions);

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
			return new ScriptObject(TypeInfo, this->Interpreter);
		});
	}
}
