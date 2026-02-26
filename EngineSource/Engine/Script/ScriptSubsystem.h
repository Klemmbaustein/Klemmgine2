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

	/**
	 * @brief
	 * The subsystem managing the scripting language features.
	 */
	class ScriptSubsystem : public subsystem::Subsystem
	{
	public:

		/// Language runtime, running the compiled bytecode
		ds::LanguageRuntime* Runtime = nullptr;
		/// Language context, holding the loaded script modules.
		ds::LanguageContext* ScriptLanguage = nullptr;

		/// The currently loaded script instructions
		ds::BytecodeStream* ScriptInstructions = nullptr;

		/// A list of tasks that are waiting for a timer to expire. (like an `await wait(1.0)` call)
		std::list<WaitTask> WaitTasks;

		/// Script provider, giving the engine a list of scripts to compile.
		ScriptProvider* Scripts = nullptr;

		ScriptSubsystem();
		virtual ~ScriptSubsystem() override;

		virtual void RegisterCommands(ConsoleSubsystem* System) override;

		virtual void Update() override;

		/// Holds script type IDs of various engine types for use with runtime type detection.
		EngineModuleData ScriptEngine;

		/// The currently active script subsystem.
		static ScriptSubsystem* Instance;

		/// All script types that derive from engine::ScriptObject mapped to their engine object type IDs
		std::map<uint32, ObjectTypeID> ScriptObjectIds;

		/// The last UI parse data and class mappings.
		ui::UIParseData UIData;

		/// The dynamic markup context to load KlemmUI markup code with.
		kui::markup::DynamicMarkupContext UIContext;

		/**
		 * @brief
		 * Recompiles all scripts and reloads all active script objects.
		 *
		 * Compilation errors are logged to the console.
		 *
		 * @return
		 * True if no errors occurred, false if something went wrong (like a script compile error)
		 */
		bool Reload();

		ds::RuntimeClass* GetClassFromObject(SceneObject* Object);
		void RegisterClassForObject(SceneObject* Object, ds::RuntimeClass* Class);

		/// Maps all objects to their script language representation. This might be a user-defined class for an
		/// object defined in the scripts or
		std::map<SceneObject*, ds::RuntimeClass*> ScriptObjectMappings;
		std::map<void*, ds::RuntimeClass*> UIObjectMappings;

	private:
		void ReloadDynamicUIContext();
	};
}