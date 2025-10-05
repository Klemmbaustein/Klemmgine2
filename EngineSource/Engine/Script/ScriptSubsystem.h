#pragma once
#include <Engine/Subsystem/Subsystem.h>

namespace ds
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

		ds::InterpretContext* Interpreter = nullptr;
		ds::LanguageContext* ScriptLanguage = nullptr;

		ds::BytecodeStream* ScriptInstructions = nullptr;

		ScriptSubsystem();
		virtual ~ScriptSubsystem() override;

		virtual void RegisterCommands(subsystem::ConsoleSubsystem* System) override;

		void Reload();
	};
}