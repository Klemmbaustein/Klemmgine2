#include "ScriptSubsystem.h"
#include "ScriptObject.h"
#include "EngineModules.h"
#include <Engine/Subsystem/ConsoleSubsystem.h>
#include <language.hpp>
#include <modules/standardLibrary.hpp>
#include <Engine/Debug/TimeLogger.h>
#include <filesystem>

using namespace lang;

engine::script::ScriptSubsystem::ScriptSubsystem()
	: subsystem::Subsystem("Script", Log::LogColor::Yellow)
{
	this->ScriptLanguage = new LanguageContext();
	modules::registerStandardLibrary(this->ScriptLanguage);
	RegisterEngineModules(this->ScriptLanguage);

	ScriptInstructions = new BytecodeStream();

	Reload();
}

engine::script::ScriptSubsystem::~ScriptSubsystem()
{
}

void engine::script::ScriptSubsystem::Update()
{
}

void engine::script::ScriptSubsystem::RenderUpdate()
{
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
	if (this->Interpreter)
	{
		delete this->Interpreter;
	}

	auto Compiler = this->ScriptLanguage->createCompiler();

	debug::TimeLogger CompileTime = { "Compiled scripts", this->GetLogPrefixes() };

	Compiler->errors.writeError = [this](string Message)
	{
		Print(Message, LogType::Error);
	};

	for (auto& i : std::filesystem::recursive_directory_iterator("Scripts/"))
	{
		Compiler->addFile(i.path().string());
	}

	*ScriptInstructions = Compiler->compile();

	this->Interpreter = this->ScriptLanguage->createInterpreter();
	this->Interpreter->loadBytecode(ScriptInstructions);

	for (auto& [Id, TypeInfo] : ScriptInstructions->reflect.types)
	{
		Reflection::RegisterObject(TypeInfo.name, [&TypeInfo, this] -> SceneObject* {
			return new ScriptObject(TypeInfo, this->Interpreter);
		});
	}
}
