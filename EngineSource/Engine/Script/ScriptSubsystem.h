#pragma once
#include <Engine/Subsystem/Subsystem.h>

namespace lang
{
	class InterpretContext;
	struct LanguageContext;
	struct BytecodeStream;
}

namespace engine::script
{
	class ScriptSubsystem : public subsystem::Subsystem
	{
	public:

		lang::InterpretContext* Interpreter = nullptr;
		lang::LanguageContext* ScriptLanguage = nullptr;

		lang::BytecodeStream* ScriptInstructions = nullptr;

		ScriptSubsystem();
		virtual ~ScriptSubsystem() override;

		virtual void RegisterCommands(subsystem::ConsoleSubsystem* System) override;

		void Reload();
	};
}