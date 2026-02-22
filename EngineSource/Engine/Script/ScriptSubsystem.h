#pragma once
#include <Engine/Subsystem/Subsystem.h>
#include <list>
#include "ScriptProvider.h"
#include "UI/ParseUI.h"
#include "EngineModules.h"
#include "kui/DynamicMarkup.h"
#include <Engine/Objects/Reflection/ObjectReflection.h>
#include <map>

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

		EngineModuleData ScriptEngine;

		static ScriptSubsystem* Instance;

		std::map<uint32, ObjectTypeID> ScriptObjectIds;

		ui::UIParseData UIData;
		kui::markup::DynamicMarkupContext UIContext;

		bool Reload();

		ds::RuntimeClass* GetClassFromObject(SceneObject* Object);
		void RegisterClassForObject(SceneObject* Object, ds::RuntimeClass* Class);

		std::map<SceneObject*, ds::RuntimeClass*> ScriptObjectMappings;
		std::map<void*, ds::RuntimeClass*> UIObjectMappings;

	private:
		void ReloadDynamicContext();
	};
}