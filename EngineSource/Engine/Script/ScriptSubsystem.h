#pragma once
#include <Engine/Subsystem/Subsystem.h>
#include <list>
#include "ScriptProvider.h"

namespace ds
{
	class InterpretContext;
	class LanguageRuntime;
	struct LanguageContext;
	struct BytecodeStream;
	struct RuntimeClass;
}

namespace engine::script
{
	struct WaitTask
	{
		ds::RuntimeClass* TaskObject;
		float Time = 0;
	};

	class ScriptSubsystem : public subsystem::Subsystem
	{
	public:

		ds::LanguageRuntime* Runtime = nullptr;
		ds::LanguageContext* ScriptLanguage = nullptr;

		ds::BytecodeStream* ScriptInstructions = nullptr;

		std::list<WaitTask> WaitTasks;

		ScriptProvider* Scripts = nullptr;

		ScriptSubsystem();
		virtual ~ScriptSubsystem() override;

		virtual void RegisterCommands(ConsoleSubsystem* System) override;

		virtual void Update() override;

		void Reload();
	};
}