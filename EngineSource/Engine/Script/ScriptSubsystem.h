#pragma once
#include <Engine/Subsystem/Subsystem.h>
#include <list>

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

		ScriptSubsystem();
		virtual ~ScriptSubsystem() override;

		virtual void RegisterCommands(subsystem::ConsoleSubsystem* System) override;

		virtual void Update() override;

		void Reload();
	};
}